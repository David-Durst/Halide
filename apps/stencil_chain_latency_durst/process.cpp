#include <chrono>
#include <cstdio>
#include <cstdlib>

#include "stencil_chain.h"
#ifndef NO_AUTO_SCHEDULE
#include "stencil_chain_auto_schedule.h"
#endif

#include "HalideBuffer.h"
#include "halide_benchmark.h"
#include "halide_image_io.h"

using namespace Halide::Runtime;
using namespace Halide::Tools;

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: ./process x_size\n"
               "e.g.: ./process 100\n");
        return 0;
    }

    int x_size = std::stoi(argv[1]);
    printf("parsed arg\n");

    Halide::Runtime::Buffer<uint16_t> input(x_size, x_size);
    for (int x = 0; x < x_size; x++) {
      for (int y = 0; y < x_size; y++) {
        input(x,y) = rand() % 65535;
      }
    }
    printf("made input\n");

    Halide::Runtime::Buffer<uint16_t> output(x_size, x_size);
    printf("made output\n");

    stencil_chain(input, output);

    // Timing code

    // Manually-tuned version
    double best_manual = benchmark(10, 10, [&]() {
        stencil_chain(input, output);
        output.device_sync();
    });
    printf("Manually-tuned time: %g ms\n", best_manual * 1e3);

#ifndef NO_AUTO_SCHEDULE
    // Auto-scheduled version
    double best_auto = benchmark(10, 10, [&]() {
        stencil_chain_auto_schedule(input, output);
        output.device_sync();
    });
    printf("Auto-scheduled time: %g ms\n", best_auto * 1e3);
#endif

    printf("Success!\n");
    return 0;
}
