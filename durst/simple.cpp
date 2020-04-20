// On linux, you can compile and run it like so:
// g++ simple.cpp -g -I ../include -L ../bin -lHalide -lpthread -ldl -o simple -std=c++11
// LD_LIBRARY_PATH=../bin ./simple

// On os x:
// g++ simple.cpp -g -I ../include -L ../bin -lHalide -o simple -std=c++11
// DYLD_LIBRARY_PATH=../bin ./simple

// The only Halide header file you need is Halide.h. It includes all of Halide.
#include "Halide.h"
#include "helpers.h"

// We'll also include stdio for printf.
#include <stdio.h>
#include <typeinfo>

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

    //Halide::Target target = Halide::get_host_target();

    //Halide::Module module = gradient.compile_to_module({}, "", target);

    auto bounds = gradient.function().schedule().bounds();
    print_bounds(bounds);

    return 0;
}
