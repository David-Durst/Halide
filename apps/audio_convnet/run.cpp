// Include the generated header
#include "bin/convnet.h"

// For std::sin
#include <cmath>

// For std::stoi
#include <cstdlib>

// For Halide::Runtime::Buffer
#include "HalideBuffer.h"

// For benchmarking
#include "halide_benchmark.h"

#include <ctime>
#include <iostream>

auto last_samples_time = Halide::Tools::benchmark_now();

extern "C" int provide_more_samples(halide_buffer_t *buffer) {
    if (buffer->is_bounds_query()) {
        // This feature of Halide is not relevant to this problem.
        return 0;
    }

    const int min_t = buffer->dim[1].min;
    const int max_t = min_t + buffer->dim[1].extent;

    //printf("Providing samples %d .. %d\n", min_t, max_t);
    for (int i = 0; i < buffer->dim[1].extent; i++) {
        int timestamp = min_t + i;
        buffer->host[i] = 0; // std::sin(0.001 * timestamp);
    }

    last_samples_time = Halide::Tools::benchmark_now();

    return 0;
}

int main(int argc, char **argv) {
    if (argc < 4) {
        printf("Usage: ./bin num_channels num_samples num_layers\n");
        return 0;
    }

    int channels = std::stoi(argv[1]);
    int samples = std::stoi(argv[2]);
    int layers = std::stoi(argv[3]);

    // Use channels-channel output
    Halide::Runtime::Buffer<uint16_t> output(nullptr, channels, samples);

    // Give the pipeline a chance to increase the output size
    convnet(output);
    std::cout << "Samples used: " << output.dim(1).extent() << "\n";
    output.allocate();

    // Warm up
    convnet(output);
    auto start_time = Halide::Tools::benchmark_now();
    convnet(output);
    auto end_time = Halide::Tools::benchmark_now();
    double delta_time = Halide::Tools::benchmark_duration_seconds(last_samples_time, end_time) * 1e3;
    double t = Halide::Tools::benchmark_duration_seconds(start_time, end_time) * 1e3;

    //printf("Time: %f seconds from %d samples\n", t, num_samples);
    //std::cout << "start time: " << last_samples_time << std::endl;
    std::cout << "Latency: " << delta_time << std::endl;
    std::cout << "Time: " << t << std::endl;
    //std::cout << "delta: " << delta_time  << std::endl;
    //std::cout << "float division approach: " << ((float) delta_time)  / CLOCKS_PER_SEC * 1000 << std::endl;
    //std::cout << "long double division approach: " << ((long double) delta_time)  / CLOCKS_PER_SEC * 1000 << std::endl;
    //printf("Latency: %f ms\n", ((double) delta_time) / CLOCKS_PER_SEC * 1e3);
    //printf("Throughput: %f samples per second\n", output.dim(1).extent() / t);

    // The network has 32 layers, where each layer does 3 x 256 x 256
    // multiply-adds. A multiply-add is typically counted as 2 flops.
    size_t total_flops = channels * channels * 3 * layers * 2;
    //total_flops *= output.dim(1).extent();
    total_flops *= samples;
    printf("Throughput: %f GOps\n", (total_flops / (t / 1000 * 1000000000)));

    return 0;
}
