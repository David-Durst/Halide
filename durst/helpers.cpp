#include "Halide.h"

/** print a bounds object. */
void print_bounds(const std::vector<Halide::Internal::Bound> &bounds) {
  for (auto x : bounds) {
    std::cout << "min: " << x.min << ", extent: " << x.extent <<
      ", modulus " << x.modulus << ", remainder " << x.remainder << std::endl;
  }
}
