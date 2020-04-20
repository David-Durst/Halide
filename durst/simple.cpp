// On linux, you can compile and run it like so:
// g++ simple.cpp -g -I ../include -L ../bin -lHalide -lpthread -ldl -o simple -std=c++11
// LD_LIBRARY_PATH=../bin ./simple

// On os x:
// g++ simple.cpp -g -I ../include -L ../bin -lHalide -o simple -std=c++11
// DYLD_LIBRARY_PATH=../bin ./simple

// The only Halide header file you need is Halide.h. It includes all of Halide.
#include "Halide.h"

// We'll also include stdio for printf.
#include <stdio.h>

int main(int argc, char **argv) {

    // This program defines a single-stage imaging pipeline that
    // outputs a grayscale diagonal gradient.

    Halide::Func gradient;

    // Var objects are names to use as variables in the definition of
    // a Func. They have no meaning by themselves.
    Halide::Var x, y;

    gradient(x, y) = x+y;

    gradient.bound(x, 0, 400);
    gradient.bound(y, 0, 400);

    /*
    Halide::Buffer<int32_t> output = gradient.realize(800, 600);

    // Let's check everything worked, and we got the output we were
    // expecting:
    for (int j = 0; j < output.height(); j++) {
        for (int i = 0; i < output.width(); i++) {
            // We can access a pixel of an Buffer object using similar
            // syntax to defining and using functions.
            if (output(i, j) != i + j) {
                printf("Something went wrong!\n"
                       "Pixel %d, %d was supposed to be %d, but instead it's %d\n",
                       i, j, i + j, output(i, j));
                return -1;
            }
        }
    }
    */

    Halide::Target target = Halide::get_host_target();

    Halide::Module module = gradient.compile_to_module({}, "", target);

    std::cout << module << std::endl;

    // Everything worked! We defined a Func, then called 'realize' on
    // it to generate and run machine code that produced an Buffer.
    //printf("Success!\n");

    return 0;
}
