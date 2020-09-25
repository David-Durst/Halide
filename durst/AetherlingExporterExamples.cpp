// On linux, you can compile and run it like so:
// g++ *.cpp -g -I ../include -I ../tools -L ../bin -lHalide `libpng-config --cflags --ldflags` -ljpeg  -lpthread -ldl -o SourceExporterExamples -std=c++11
// LD_LIBRARY_PATH=../bin ./SourceExporterExamples

// On os x:
// g++ *.cpp -g -I ../include -L ../bin -lHalide -o SourceExporterExamples -std=c++11
// DYLD_LIBRARY_PATH=../bin ./SourceExporterExamples

#include "Halide.h"
#include "AetherlingExporter.h"
#include "halide_image_io.h"
#include <stdio.h>

void brighten() {
  Halide::Func brighter;

  Halide::Var x,y,c;

  Halide::Buffer<uint8_t> input = Halide::Tools::load_image("../tutorial/images/rgb.png");

  brighter(x,y,c) = input(x,y,c) + 1;

  /*
  std::cout << "BRIGHTEN EXAMPLE" << std::endl;
  std::cout << "printing input" << std::endl;
  std::cout << input << std::endl;
  std::cout << "printing brighter" << std::endl;
  std::cout << brighter << std::endl;
  std::cout << "printing brighter with dependencies" << std::endl;
  */
  Halide::export_func(std::cout, brighter, std::vector<int>{8,8,3});
  
}

void down() {
  Halide::Func down, downplus1;

  Halide::Var x,y,c;

  Halide::Buffer<uint8_t> input = Halide::Tools::load_image("../tutorial/images/rgb.png");

  down(x,y,c) = input(2*x,2*y,c);
  downplus1(x,y,c) = down(x,y,c)+1;

  /*
  std::cout << "DOWN EXAMPLE" << std::endl;
  std::cout << "printing gradient" << std::endl;
  std::cout << gradient << std::endl;
  std::cout << "printing down" << std::endl;
  std::cout << down << std::endl;
  std::cout << "printing downplus1" << std::endl;
  std::cout << downplus1 << std::endl;
  std::cout << "printing downplus1 with dependencies" << std::endl;
  */
  Halide::export_func(std::cout, downplus1, std::vector<int>{8,8,3});
}

void up() {
  Halide::Func up, upplus1;

  Halide::Var x,y,c;

  Halide::Buffer<uint8_t> input = Halide::Tools::load_image("../tutorial/images/rgb.png");

  up(x,y,c) = input(x/2,y/2,c);
  upplus1(x,y,c) = up(x,y,c)+1;

  /*
  std::cout << "UP EXAMPLE" << std::endl;
  std::cout << "printing gradient" << std::endl;
  std::cout << gradient << std::endl;
  std::cout << "printing up" << std::endl;
  std::cout << up << std::endl;
  std::cout << "printing upplus1" << std::endl;
  std::cout << upplus1 << std::endl;
  std::cout << "printing upplus1 with dependencies" << std::endl;
  */
  Halide::export_func(std::cout, upplus1, std::vector<int>{8,8,3});
}


void conv() {
  Halide::Func conv;

  Halide::RDom r(-1, 3, -1, 3);

  Halide::Var x,y,c;

  Halide::Buffer<uint8_t> input = Halide::Tools::load_image("../tutorial/images/rgb.png");

  //Halide::Func clamped = Halide::BoundaryConditions::repeat_edge(gradient);

  conv(x,y,c) = input(x - 1, y - 1, c) + input(x, y - 1, c) +
    input(x - 1, y, c) + input(x, y, c);


  /*
  std::cout << "CONV EXAMPLE" << std::endl;
  std::cout << "printing gradient" << std::endl;
  std::cout << gradient << std::endl;
  //std::cout << "printing clamped" << std::endl;
  //std::cout << clamped << std::endl;
  std::cout << "printing conv" << std::endl;
  std::cout << conv << std::endl;
  std::cout << "printing conv with dependenceis" << std::endl;
  */
  Halide::export_func(std::cout, conv, std::vector<int>{8,8,3});
}

int main(int argc, char **argv) {
  //add();
  brighten();
  down();
  up();
  conv();
  return 0;
}
