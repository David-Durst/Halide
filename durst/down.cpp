// On linux, you can compile and run it like so:
// g++ down.cpp -g -I ../include -L ../bin -lHalide -lpthread -ldl -o down -std=c++11
// LD_LIBRARY_PATH=../bin ./down

// On os x:
// g++ down.cpp -g -I ../include -L ../bin -lHalide -o down -std=c++11
// DYLD_LIBRARY_PATH=../bin ./down

// The only Halide header file you need is Halide.h. It includes all of Halide.
#include "Halide.h"

// We'll also include stdio for printf.
#include <stdio.h>

int main(int argc, char **argv) {

    // This program defines a single-stage imaging pipeline that
    // outputs a grayscale diagonal gradient.

    Halide::Func gradient, down;

    // Var objects are names to use as variables in the definition of
    // a Func. They have no meaning by themselves.
    Halide::Var x, y;

    gradient(x, y) = x+y;

    gradient.bound(x, 0, 400);
    gradient.bound(y, 0, 400);

    down(x,y) = gradient(2*x,2*y);

    gradient.compute_at(gradient, x);

    Halide::Target target = Halide::get_host_target();

    Halide::Module module = down.compile_to_module({}, "", target);

    std::cout << module << std::endl;

    return 0;
}
