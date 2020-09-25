#include "ClockworkExporter.h"
#include "Halide.h"
#include <fstream>
#include <iostream>
#include <math.h>

namespace {

std::vector<Halide::Func> func_vector(const std::string &name, int size) {
    std::vector<Halide::Func> funcs;
    for (int i = 0; i < size; i++) {
        funcs.emplace_back(Halide::Func{name + "_" + std::to_string(i)});
    }
    return funcs;
}

class Exposure : public Halide::Generator<Exposure> {
public:
    GeneratorParam<int> x_max_param{"x_max_param", 1536};
    GeneratorParam<int> y_max_param{"y_max_param", 2560};

    Input<Buffer<uint16_t>> input{"input", 2};
    Output<Buffer<uint16_t>> output{"output", 2};

    void generate() {
        int x_max = (int)x_max_param;
        int y_max = (int)y_max_param;
        int levels = log(x_max * y_max) / log(4);
        Var x("x"), y("y"), c("c");

        RDom r_3x3(-1, 3, -1, 3), r_2x2(-1, 2, -1, 2);

        auto gaussian_pyramid = func_vector("gaussian_pyramid", levels);
        auto laplacian_pyramid = func_vector("laplacian_pyramid", levels);
        auto merged_pyramid = func_vector("merged_pyramid", levels);
        auto collapsed_pyramid = func_vector("collapsed_pyramid", levels);

        Func clamped = Halide::BoundaryConditions::repeat_edge(input);
        Func bright, dark, bright_weights, dark_weights, weight_sums,
            bright_weights_normed, dark_weights_normed;

        bright(x, y) = clamped(x, y);
        dark(x, y) = clamped(x, y) * 3;
        bright_weights(x, y) = 2 * bright(x, y);
        dark_weights(x, y) = 2 * dark(x, y);

        // compute normalized weights
        weight_sums(x, y) = dark_weights(x, y) + bright_weights(x, y);
        //bright_weights_normed(x, y) = cast<int32_t>(bright_weights(x, y) / weight_sums(x, y));
        dark_weights_normed(x, y) = ((256 * dark_weights(x, y)) / weight_sums(x, y));

        // Compute Gaussian pyramids. The channels are dark pixel, bright pixel, and dark weights

        gaussian_pyramid[0](x, y, c) = mux(c, {dark(x, y), bright(x, y), dark_weights_normed(x, y)});
        for (int l = 1; l < levels; l++) {
            gaussian_pyramid[l](x, y, c) = cast<uint16_t>(0);
            gaussian_pyramid[l](x, y, c) +=
                gaussian_pyramid[l - 1](x * 2 + r_3x3.x, y * 2 + r_3x3.y, c);
            gaussian_pyramid[l](x, y, c) /= cast<uint16_t>(9);
        }

        laplacian_pyramid[levels - 1](x, y, c) = gaussian_pyramid[levels - 1](x, y, c);
        for (int l = 0; l < levels - 1; l++) {
            laplacian_pyramid[l](x, y, c) =
                gaussian_pyramid[l](x, y, c) -
                gaussian_pyramid[l + 1](x / 2, y / 2, c);
        }

        // merge layers
        for (int l = 0; l < levels; l++) {
            merged_pyramid[l](x, y) =
                (laplacian_pyramid[l](x, y, 0) * gaussian_pyramid[l](x, y, 2) +
                 laplacian_pyramid[l](x, y, 1) * (256 - gaussian_pyramid[l](x, y, 2))) /
                256;
        }

        collapsed_pyramid[levels - 1](x, y) = merged_pyramid[levels - 1](x, y);
        for (int l = levels - 2; l >= 0; l--) {
            collapsed_pyramid[l](x, y) =
                collapsed_pyramid[l + 1](x / 2, y / 2) + merged_pyramid[l](x, y);
            std::cerr << "handling level " << l << std::endl;
        }

        std::cerr << "collapsed pyramids" << std::endl;

        // Schedule
        if (auto_schedule) {
            output = collapsed_pyramid[0];
        } else {
            // ??ms on a 2060 RTX
            Var yo, yi, xo, xi, ci, xii, yii;
            if (get_target().has_gpu_feature()) {
                for (int l = 1; l < levels; l++) {
                    if (l == 1) {
                        gaussian_pyramid[l]
                            .in()
                            .compute_root()
                            .reorder(c, x, y)
                            .unroll(c)
                            .gpu_tile(x, y, xi, yi, 64, 16, TailStrategy::RoundUp)
                            .split(xi, xi, xii, 4)
                            .unroll(xii);

                        gaussian_pyramid[l]
                            .reorder(c, x, y)
                            .unroll(c)
                            .unroll(x);
                        gaussian_pyramid[l]
                            .update(0)
                            .unroll(r_3x3.x)
                            .unroll(r_3x3.y)
                            .reorder(c, x, y)
                            .unroll(c)
                            .unroll(x);
                        gaussian_pyramid[l]
                            .update(1)
                            .reorder(c, x, y)
                            .unroll(c)
                            .unroll(x);
                    } else {
                        gaussian_pyramid[l]
                            .in()
                            .compute_root()
                            .gpu_tile(x, y, xi, yi, 16, 16, TailStrategy::RoundUp)
                            .gpu_blocks(x, y, c);

                        gaussian_pyramid[l]
                            .update(0)
                            .unroll(r_3x3.x)
                            .unroll(r_3x3.y);
                    }
                    if (l < 4) {
                        collapsed_pyramid[l]
                            .compute_at(collapsed_pyramid[0], x)
                            .gpu_threads(x, y);
                    }
                }

                {
                    // Fork the entire sub-DAG that processes the
                    // input for the first stage so that we can
                    // schedule it independently. Lots of redundant
                    // recompute.
                    Func g0 = gaussian_pyramid[0].clone_in(gaussian_pyramid[1]);
                    Func dwn = dark_weights_normed.clone_in(g0);
                    Func ws = weight_sums.clone_in(dwn);
                    Func dw = dark_weights.clone_in({ws, dwn});
                    Func bw = bright_weights.clone_in(ws);
                    Func d = dark.clone_in({dw, g0});
                    Func b = bright.clone_in({bw, g0});
                    Func c = clamped.clone_in({d, b});
                    c.compute_at(gaussian_pyramid[1].in(), x)
                        .tile(Halide::_0, Halide::_1, x, y, xi, yi, 9, 3, TailStrategy::RoundUp)
                        .unroll(xi)
                        .unroll(yi)
                        .gpu_threads(x, y)
                        .align_bounds(Halide::_0, 8, 1);

                    c.in()
                        .compute_at(gaussian_pyramid[1].in(), xi)
                        .unroll(Halide::_1)
                        .vectorize(Halide::_0);
                }

                clamped.compute_at(collapsed_pyramid[0], x)
                    .tile(Halide::_0, Halide::_1, x, y, xi, yi, 2, 2, TailStrategy::RoundUp)
                    .unroll(xi)
                    .unroll(yi)
                    .gpu_threads(x, y);

                collapsed_pyramid[0]
                    .bound(x, 0, input.width())
                    .bound(y, 0, input.height())
                    .tile(x, y, xi, yi, 32, 32, TailStrategy::RoundUp)
                    .tile(xi, yi, xii, yii, 2, 2)
                    .gpu_blocks(x, y)
                    .gpu_threads(xi, yi)
                    .vectorize(xii)
                    .unroll(yii);

                output = collapsed_pyramid[0];
            } else {
            }
        }

        // Estimates (for autoscheduler; ignored otherwise)
        {
            input.dim(0).set_estimate(0, x_max);
            input.dim(1).set_estimate(0, y_max);
            output.dim(0).set_estimate(0, x_max);
            output.dim(1).set_estimate(0, y_max);
        }

        std::ofstream memory_s("durst_memory.cpp"), memory_h_s("durst_memory.h"),
            compute_s("durst_compute.h");
        Halide::Internal::print_clockwork(memory_s, memory_h_s, compute_s, collapsed_pyramid[0],
                                          "durst_memory.h", "durst_compute.h",
                                          {x_max, y_max}, 16);

        std::cerr << "done print clockwork" << std::endl;
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(Exposure, exposure)
