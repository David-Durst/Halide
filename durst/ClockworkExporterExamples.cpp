// On linux, you can compile and run it like
// g++ *.cpp -g -I ../include -I ../tools -L ../bin -lHalide `libpng-config --cflags --ldflags` -ljpeg  -lpthread -ldl -o SourceExporterExamples -std=c++11
// LD_LIBRARY_PATH=../bin ./SourceExporterExamples

// On os x:
// g++ *.cpp -g -I ../include -L ../bin -lHalide -o SourceExporterExamples -std=c++11
// DYLD_LIBRARY_PATH=../bin ./SourceExporterExamples

#include "Halide.h"
#include "ClockworkExporter.h"
#include "halide_image_io.h"
#include <stdio.h>
#include <iostream>
#include <fstream>

const std::string MEM_FILE = "durst_memory.cpp";
const std::string MEM_H_FILE = "durst_memory.h";
const std::string COMPUTE_FILE = "durst_compute.h";

void brighten(std::ostream &mem_s, std::ostream &mem_h_s,
              std::ostream &compute_s, int parallelism) {
  Halide::Func brighter;

  Halide::Var x,y,c;

  Halide::Buffer<uint8_t> input = Halide::Tools::load_image("../tutorial/images/rgb.png");

  brighter(x,y,c) = input(x,y,c) + 1;

  brighter.realize(10,10,3);

  /*
  std::cout << "BRIGHTEN EXAMPLE" << std::endl;
  std::cout << "printing input" << std::endl;
  std::cout << input << std::endl;
  std::cout << "printing brighter" << std::endl;
  std::cout << brighter << std::endl;
  std::cout << "printing brighter with dependencies" << std::endl;
  */
  Halide::Internal::print_clockwork(mem_s, mem_h_s, compute_s, brighter,
                                    MEM_H_FILE, COMPUTE_FILE, {1000, 1000, 3},
                                    parallelism);
  
}

void edge_case(std::ostream &mem_s, std::ostream &mem_h_s,
               std::ostream &compute_s, int parallelism) {
  Halide::Buffer<uint8_t> input = Halide::Tools::load_image("../tutorial/images/rgb.png");
  Halide::Func f = Halide::BoundaryConditions::repeat_edge(input, 0, 1000, 0, 1000);

  Halide::Internal::print_clockwork(mem_s, mem_h_s, compute_s, f,
                                    MEM_H_FILE, COMPUTE_FILE, {1000, 1000, 3},
                                    parallelism);
}

void brighten_two_inputs(std::ostream &mem_s, std::ostream &mem_h_s,
                         std::ostream &compute_s, int parallelism) {
  Halide::Func brighter_first_input, brighter_second_input;

  Halide::Var x,y,c;

  Halide::Buffer<uint8_t> input1 = Halide::Tools::load_image("../tutorial/images/rgb.png");
  Halide::Buffer<uint8_t> input2 = Halide::Tools::load_image("../tutorial/images/rgb.png");

  brighter_first_input(x,y,c) = input1(x,y,c) + 1;
  brighter_second_input(x,y,c) = brighter_first_input(x,y,c) + input2(x,y,c);

  Halide::Internal::print_clockwork(mem_s, mem_h_s, compute_s, brighter_second_input,
                                    MEM_H_FILE, COMPUTE_FILE, {1000, 1000, 3},
                                    parallelism);
  
}

void down(std::ostream &mem_s, std::ostream &mem_h_s,
          std::ostream &compute_s, int parallelism) {
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
  Halide::Internal::print_clockwork(mem_s, mem_h_s, compute_s, downplus1,
                                    MEM_H_FILE, COMPUTE_FILE, {1000, 1000, 3},
                                    parallelism);
}

void up(std::ostream &mem_s, std::ostream &mem_h_s,
        std::ostream &compute_s, int parallelism) {
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
  Halide::Internal::print_clockwork(mem_s, mem_h_s, compute_s, upplus1,
                                    MEM_H_FILE, COMPUTE_FILE, {1000, 1000, 3},
                                    parallelism);
}

void down_up(std::ostream &mem_s, std::ostream &mem_h_s,
          std::ostream &compute_s, int parallelism) {
  Halide::Func downx, down, downplus1, updownx, updown, updownsubin;

  Halide::Var x,y,c;

  Halide::Buffer<uint8_t> input = Halide::Tools::load_image("../tutorial/images/rgb.png");

  downx(x,y,c) = input(2*x,y,c) + input(2*x - 1,y,c) + input(2*x + 1,y,c);
  down(x,y,c) = downx(x,2*y,c) + downx(x,2*y - 1,c) + downx(x,2*y + 1,c);
  downplus1(x,y,c) = down(x,y,c)+1;
  updownx(x,y,c) = downplus1(x/2,y,c) + downplus1(x/2 + 1,y,c) + downplus1(x/2 - 1,y,c);
  updown(x,y,c) = updownx(x,y/2,c) + updownx(x,y/2 + 1,c) + updownx(x,y/2 - 1,c);
  updownsubin(x,y,c) = updown(x,y,c)-input(x,y,c);

  Halide::Internal::print_clockwork(mem_s, mem_h_s, compute_s, updownsubin,
                                    MEM_H_FILE, COMPUTE_FILE, {1000, 1000, 3},
                                    parallelism);
}


void conv(std::ostream &mem_s, std::ostream &mem_h_s,
          std::ostream &compute_s, int parallelism) {
  Halide::Func durst_conv;

  Halide::RDom r(-1, 3, -1, 3);

  Halide::Var x,y,c;

  Halide::Buffer<uint8_t> input = Halide::Tools::load_image("../tutorial/images/rgb.png");
  Halide::Func f = Halide::BoundaryConditions::repeat_edge(input, 0, 1000, 0, 1000);

  //Halide::Func clamped = Halide::BoundaryConditions::repeat_edge(gradient);

  durst_conv(x,y,c) = f(x - 1, y - 1, c) + f(x, y - 1, c) +
    f(x - 1, y, c) + f(x, y, c);
  //durst_conv(x,y,c) = input(x - 1, y - 1, c) + input(x, y - 1, c) +
  //  input(x - 1, y, c) + input(x, y, c);


  /*
  std::cout << "CONV EXAMPLE" << std::endl;
  std::cout << "printing gradient" << std::endl;
  std::cout << gradient << std::endl;
  //std::cout << "printing clamped" << std::endl;
  //std::cout << clamped << std::endl;
  std::cout << "printing conv" << std::endl;
  std::cout << conv << std::endl;
  std::cout << "printing conv with dependencies" << std::endl;
  */
  Halide::Internal::print_clockwork(mem_s, mem_h_s, compute_s, durst_conv,
                                    MEM_H_FILE, COMPUTE_FILE, {1000, 1000, 3},
                                    parallelism);
}

void conv_update(std::ostream &mem_s, std::ostream &mem_h_s,
          std::ostream &compute_s, int parallelism) {
  Halide::Func durst_conv_update;

  Halide::RDom r(-1, 3, -1, 3);

  Halide::Var x,y,c;

  Halide::Buffer<uint8_t> input = Halide::Tools::load_image("../tutorial/images/rgb.png");
  Halide::Func f = Halide::BoundaryConditions::repeat_edge(input, 0, 1000, 0, 1000);

  durst_conv_update(x,y,c) = 0;
  durst_conv_update(x,y,c) += f(x + r.x, y + r.y, c);

  Halide::Internal::print_clockwork(mem_s, mem_h_s, compute_s, durst_conv_update,
                                    MEM_H_FILE, COMPUTE_FILE, {1000, 1000, 3},
                                    parallelism);
}

void conv_update_simple(std::ostream &mem_s, std::ostream &mem_h_s,
                        std::ostream &compute_s, int parallelism) {
  Halide::Func durst_conv_update;

  Halide::RDom r(-1, 3, -1, 3);

  Halide::Var x,y,c;

  Halide::Buffer<uint8_t> input = Halide::Tools::load_image("../tutorial/images/rgb.png");

  durst_conv_update(x,y,c) = 0;
  durst_conv_update(x,y,c) += input(x + r.x, y + r.y, c);

  Halide::Internal::print_clockwork(mem_s, mem_h_s, compute_s, durst_conv_update,
                                    MEM_H_FILE, COMPUTE_FILE, {1000, 1000, 3},
                                    parallelism);
}

int main(int argc, char **argv) {
  //add();
  std::ofstream mem_s(MEM_FILE), mem_h_s(MEM_H_FILE),
    compute_s(COMPUTE_FILE);
  //brighten(mem_s, mem_h_s, compute_s, 1);
  edge_case(mem_s, mem_h_s, compute_s, 2);
  //brighten_two_inputs(mem_s, mem_h_s, compute_s, 1);
  //down(mem_s, mem_h_s, compute_s, 2);
  //up(mem_s, mem_h_s, compute_s, 4);
  //down_up(mem_s, mem_h_s, compute_s, 1);
  conv(mem_s, mem_h_s, compute_s, 4);
  //conv_update(mem_s, mem_h_s, compute_s, 1);
  conv_update_simple(mem_s, mem_h_s, compute_s, 1);
  mem_s.close();
  mem_h_s.close();
  compute_s.close();

  return 0;
}
