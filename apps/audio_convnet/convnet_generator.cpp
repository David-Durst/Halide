#include "Halide.h"

using namespace Halide;

class Convnet : public Generator<Convnet> {
public:
    GeneratorParam<int> channels_param{"channels", 256, 8, 1024};
    GeneratorParam<int> layers_param{"layers", 32, 1, 64};
    GeneratorParam<int> batch_size{"batch_size", 60, 1, 6000};
    GeneratorParam<bool> arm_arch{"arm_arch", false};
    Output<Buffer<uint16_t>> result{"result", 2};

    Var c, co, ci, t, ti;

    Func layer(Func in, Buffer<uint16_t> weights, Buffer<uint16_t> bias) {

        // Assume weights are in layout (from innermost dimension out):
        // output channels, input channels, time.

        // Passing the weights as literal buffers bakes them in at
        // compile time. If you want to switch the weights out
        // dynamically then use Input<Buffer<float>> instead.

        Func conv;
        conv(c, t) = bias(c);
        RDom r(0, weights.dim(1).extent(), 0, weights.dim(2).extent());
        conv(c, t) += weights(c, r[0], r[1]) * in(r[0], t + r[1]);

        Func sigmoid;
        //Expr y = fast_exp(cast<float>(conv(c, t)));
        sigmoid(c, t) = max(conv(c,t), 0);// cast<uint16_t>(y / (y + 1));

        // Calling code will set the store_at/compute_at for
        // sigmoid. We'll do the rest of the schedule here.

        // We'll compute tiles with ~10 accumulator registers (20 on
        // arm). If we have enough time samples in each batch we'll do
        // 5 time samples x 2 vector output channels.
        const int vec = natural_vector_size<uint16_t>();
        int tile_t = 1, tile_c = 1;

        // Tile sizes tuned to be as large as possible and as square
        // as possible without causing spills to the stack in the
        // inner loop.
        if (get_target().arch == Target::ARM) {
            // On arm-64 we have fused multiply-add, so we don't need
            // to use registers to cover multiply latencies and can
            // just use most of them. 24 seems to be fine.
            if (batch_size == 1) {
                tile_t = 1;
                tile_c = 24;
            } else if (batch_size == 2) {
                tile_t = 2;
                tile_c = 12;
            } else if (batch_size == 3) {
                tile_t = 3;
                tile_c = 8;
            } else if (batch_size == 4) {
                tile_t = 4;
                tile_c = 6;
            } else if (batch_size == 5) {
                tile_t = 5;
                tile_c = 4;
            } else if (batch_size == 6) {
                tile_t = 6;
                tile_c = 4;
            } else if (batch_size == 7) {
                tile_t = 7;
                tile_c = 3;
            } else if (batch_size == 8) {
                tile_t = 8;
                tile_c = 3;
            } else if (batch_size == 9) {
                tile_t = 3;
                tile_c = 8;
            } else if (batch_size == 10) {
                tile_t = 5;
                tile_c = 4;
            } else {
                tile_t = 6;
                tile_c = 4;
            }
        } else {
            // On x86 we don't have a fused 16-bit multiply add, so
            // the upper limit seems to be 9 registers.
            if (batch_size == 1) {
                tile_t = 1;
                tile_c = 6;
            } else if (batch_size == 2) {
                tile_t = 2;
                tile_c = 4;
            } else if (batch_size == 3) {
                tile_t = 3;
                tile_c = 3;
            } else if (batch_size == 4) {
                tile_t = 4;
                tile_c = 2;
            } else if (batch_size == 5) {
                tile_t = 5;
                tile_c = 2;
            } else if (batch_size == 6) {
                tile_t = 6;
                tile_c = 1;
            } else if (batch_size == 7) {
                tile_t = 7;
                tile_c = 1;
            } else if (batch_size == 8) {
                tile_t = 4;
                tile_c = 2;
            } else {
                tile_t = 3;
                tile_c = 3;
            }
        }

        sigmoid.tile(c, t, ci, ti, vec*tile_c, tile_t, TailStrategy::ShiftInwards)
            .vectorize(ci, vec)
            .unroll(ci)
            .unroll(ti);

        conv.compute_at(sigmoid, c)
            .vectorize(c, vec)
            .unroll(c)
            .unroll(t)
            .update()
            .vectorize(c, vec)
            .unroll(c)
            .reorder(t, c, r[1], r[0])
            .unroll(t)
            .unroll(r[1]);

        return sigmoid;
    }

    void generate() {
        int channels = (int) channels_param;
        int layers_num = (int) layers_param;

        // For the input activations make an external call to a c++
        // function to get some more samples
        Func in;
        in.define_extern("provide_more_samples", {}, {UInt(16)}, {c, t});

        std::vector<Func> layers;

        for (int i = 0; i < layers_num; i++) {

            // Assume some number of layers in order for now
            Buffer<uint16_t> weights(channels, i == 0 ? 1 : channels, 3);
            weights.fill(0.0f);

            Buffer<uint16_t> bias(channels);
            bias.fill(0.0f);

            Func prev = i == 0 ? in : layers.back();
            Func next = layer(prev, weights, bias);

            layers.push_back(next);

            if ((layers.size() & 7) == 7) {
                printf("Skip connection from %d and %d\n",
                       (int)layers.size()/2, (int)layers.size());

                // Throw in a skip connection to an earlier layer just for fun.
                Func skip;
                skip(c, t) = next(c, t) + layers[layers.size()/2](c, t);
                layers.push_back(skip);


                // Schedule for the skip layer doesn't really matter -
                // it's a tiny fraction of the overall compute.
                skip.vectorize(c, 8);
            }
        }

        result(c, t) = layers.back()(c, t);

        // Processing multiple timesteps in the inner loop lets us
        // reuse loaded weights. Use a multiple of 5, because that's
        // the tile size in time we used for the individual conv
        // layers.
        result.tile(c, t, ci, ti, 32, (int) batch_size, TailStrategy::RoundUp)
            .vectorize(ci);

        for (size_t i = 0; i < layers.size(); i++) {
            // State persists at the root level
            layers[i].store_root();

            // But computation is done lazily per timeslice of the output
            layers[i].compute_at(result, t);

            // Every layer gets its own thread, communicating with
            // other layers via circular buffers guarded by
            // semaphores. Comment out to disable parallelism. There
            // are also ways to make groups of layers share a thread
            // instead of one thread per layer, but it's a little
            // slower on my machine.
            // layers[i].async();
        }

        // Ask for more samples at this granularity. Currently very
        // fine, but doesn't really matter because the overhead of
        // this is negligible relative to the convs.
        in.compute_at(result, t);
    }
};

HALIDE_REGISTER_GENERATOR(Convnet, convnet)
