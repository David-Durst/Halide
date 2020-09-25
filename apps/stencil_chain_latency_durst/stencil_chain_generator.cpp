#include "Halide.h"
#include <stdlib.h>
#include <string>
#include <algorithm>

namespace {

class StencilChain : public Halide::Generator<StencilChain> {
public:
    GeneratorParam<int> x_size{"x_size", 10, 1, 10000};
    GeneratorParam<int> stencils{"stencils", 32, 1, 100};
    GeneratorParam<int> muladds{"muladds", 1, 1, 100};
    GeneratorParam<int> stencil_size{"stencil_size", 5, 1, 100};

    Input<Buffer<uint16_t>> input{"input", 2};
    Output<Buffer<uint16_t>> output{"output", 2};

    void generate() {

        std::vector<Func> stages;

        Var x("x"), y("y");

        Func f = Halide::BoundaryConditions::repeat_edge(input);

        stages.push_back(f);
        int sten_max = ((int) stencil_size) / 2;
        int sten_min = sten_max*(-1);

        std::cout << "sten_max: " << sten_max << std::endl;
        std::cout << "sten_min: " << sten_min << std::endl;
        std::cout << "muladds: " << (int)muladds << std::endl;

        for (int s = 0; s < (int)stencils; s++) {
            Func f("stage_" + std::to_string(s));
            Expr e = cast<uint16_t>(1);
            for (int k = 0; k < (int) muladds; k++) {
                Expr e_inner = cast<uint16_t>(0);
                for (int i = sten_min; i <= sten_max; i++) {
                    for (int j = sten_min; j <= sten_max; j++) {
                        e_inner += (1 + (rand() % 100)) * stages.back()(x + i, y + j);
                    }
                }
                e *= e_inner;
            }

            f(x, y) = e;
            stages.push_back(f);
        }
/*
        stages.back().trace_stores();
        stages.back().trace_realizations();
        get_target().with_feature( Halide::Target::TraceLoads ); //trace loads on all Funcs
        get_target().with_feature( Halide::Target::TraceStores ); //trace stores on all Funcs
        get_target().with_feature( Halide::Target::TraceRealizations ); //trace realizations on all Funcs
        */
        output(x, y) = stages.back()(x, y);

        /* ESTIMATES */
        // (This can be useful in conjunction with RunGen and benchmarks as well
        // as auto-schedule, so we do it in all cases.)
        {
            const int width = x_size;
            const int height = x_size;
            // Provide estimates on the input image
            input.set_estimates({{0, width}, {0, height}});
            // Provide estimates on the pipeline output
            output.set_estimates({{0, width}, {0, height}});
        }

        if (auto_schedule) {
            // nothing
        } else if (get_target().has_gpu_feature()) {
		std::cout << "scheduling gpu with " << (int)stencils << " stages" << std::endl;
            // GPU schedule

            // 2.9 ms on a 2060 RTX

            // It seems that just compute-rooting all the stencils is
            // fastest on this GPU, plus some unrolling and aggressive
            // staging to share loads between adjacent pixels.
            Var xi, yi, xii, yii;
            stages.pop_back();
            stages.push_back(output);
            for (size_t i = 1; i < stages.size(); i++) {
                Func &s = stages[i];
                Func prev = stages[i - 1];
                x = s.args()[0];
                y = s.args()[1];
                s.compute_root()
                    .gpu_tile(x, y, xi, yi, (32 - sten_max)*2, 12)
                    .tile(xi, yi, xii, yii, 2, 2)
                    .unroll(xii)
                    .unroll(yii);
                prev.in()
                    .compute_at(s, x)
                    .tile(prev.args()[0], prev.args()[1], xi, yi, 2, 2)
                    .vectorize(xi)
                    .unroll(yi)
                    .gpu_threads(prev.args()[0], prev.args()[1]);
                prev.in()
                    .in()
                    .compute_at(s, xi)
                    .vectorize(prev.args()[0], 2)
                    .unroll(prev.args()[0])
                    .unroll(prev.args()[1]);
            }

        } else {
            // CPU schedule
            // 4.23ms on an Intel i9-9960X using 16 threads at 3.5
            // GHz.

            // Runtime is pretty noisy, so benchmarked over 1000
            // trials instead of the default of 10 in the
            // Makefile. This uses AVX-512 instructions, but not
            // floating-point ones. My CPU seems to hover at 3.5GHz on
            // this workload.
            std::cout << "no gpu" << std::endl;

            const int vec = natural_vector_size<uint16_t>();

            // How many stencils in between each compute-root
            const int group_size = 11;
            Var yi, yo, xo, xi, t;

            const int last_stage_idx = (int)stages.size() - 1;
            for (int j = last_stage_idx; j > 0; j -= group_size) {
                Func out = (j == last_stage_idx) ? output : stages[j];

                const int stages_to_output = last_stage_idx - j;
                const int expansion = 4 * stages_to_output;
                const int w = x_size + expansion;
                const int h = x_size + expansion;

                out.compute_root()
                    // Break into 16 tiles for our 16 threads
                    .tile(x, y, xo, yo, xi, yi, w / 4, h / 4, TailStrategy::GuardWithIf)
                    .fuse(xo, yo, t)
                    .parallel(t)
                    .vectorize(xi, std::min(vec, w/4));

                for (int i = std::max(0, j - group_size + 1); i < j; i++) {
                    Func s = stages[i];
                    s.store_at(out, t)
                        .compute_at(out, yi)
                        .vectorize(s.args()[0], std::min(vec, w/4));
                }
            }
        }
    }
};

}  // namespace

HALIDE_REGISTER_GENERATOR(StencilChain, stencil_chain)
