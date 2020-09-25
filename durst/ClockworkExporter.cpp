#include "ClockworkExporter.h"
#include "Halide.h"
#include "AffineIndexingExtractor.h"
#include "VariableExtractor.h"
#include "CleanVariableNames.h"
#include "RDomExtractor.h"

#include <tuple>
#include <algorithm>

namespace Halide {
namespace Internal {

using std::map;
using std::string;
using std::vector;
using std::set;

ClockworkExporter::ClockworkExporter(std::ostream &mem_s,
                                     std::ostream &mem_h_s, 
                                     std::ostream &compute_s)
  : mem_s(mem_s), mem_h_s(mem_h_s), compute_s(compute_s){
  mem_s.setf(std::ios::fixed, std::ios::floatfield);
  mem_h_s.setf(std::ios::fixed, std::ios::floatfield);
  compute_s.setf(std::ios::fixed, std::ios::floatfield);
}

void ClockworkExporter::visit(const IntImm *op) {
  compute_s << op->value;
}

void ClockworkExporter::visit(const UIntImm *op) {
  compute_s << op->value;
}


void ClockworkExporter::visit(const Add *op) {
  op->a.accept(this);
  compute_s << " + ";
  op->b.accept(this);
}

void ClockworkExporter::visit(const Sub *op)  {
  op->a.accept(this);
  compute_s << " - ";
  op->b.accept(this);
}

void ClockworkExporter::visit(const Mul *op) {
  op->a.accept(this);
  compute_s << " * ";
  op->b.accept(this);
}

void ClockworkExporter::visit(const Div *op) {
  op->a.accept(this);
  compute_s << " / ";
  op->b.accept(this);
}

void ClockworkExporter::visit(const Variable *op) {
  compute_s << op->name;
}

/** find the affine indexes in a vector of expressions and
  * print them for a clockwork load or store statement */
void print_indexes_clockwork(const std::vector<Expr> &exprs, std::ostream &stream) {
  std::vector<Affine_Index> indexes = AffineIndexingExtractor()
    .extract_list(exprs);
  for (size_t i = 0; i < indexes.size(); i++) {
    if (i > 0) {
      stream << ", ";
    }
    indexes[i].unquote_variable = true;
    stream << "floor(" << indexes[i] << ")";
  }
}

void ClockworkExporter::visit(const Call *op) {
  mem_s << "  " << cur_op_name << "_loop_op->add_load(\"";
  // note: DON'T ADD index for number of calls here
  // the index on the buffers is the stage number
  // not the number of times buffer called in halide
  mem_s << op->name << "_buf\", \"";
  // at the same time were doing the load in the main file
  // write this variable into the expression in the helper file
  if (calls_per_update.count(op->name) == 0) {
    calls_per_update[op->name] = 0;
  }
  else {
    calls_per_update[op->name]++;
  }
  compute_s << op->name << "_" << calls_per_update[op->name];
  print_indexes_clockwork(op->args, mem_s);
  mem_s << "\");" << std::endl;
}

set<std::string> seen_vars;
void get_unique_var_name(std::string base_name, std::ostream &stream) {
  if (seen_vars.count(base_name)) {
    stream << "  " << base_name << " = prg.un(\"" << base_name << "\");" << std::endl;
  }
  else {
    stream << "  auto " << base_name << " = prg.un(\"" << base_name << "\");" << std::endl;
    seen_vars.insert(base_name);
  }

}

/** given a set of variables in iteration order (inner to outer), a name for
  * the loop in halide, and a stream to write to, create a clockwork call
  * to create a loop nest and assign that to a c++ function in the clockwork
  * code. */
std::string create_clockwork_loop_nest(vector<VarInfo> &variables,
                                       vector<RDomVarData> &rdom_vars_data,
                                       std::string halide_loop_name,
                                       std::ostream &stream) {
  for (auto const &var : variables) {
    // filter reductions becuase those will be passed in through rdom_vars_data
    if (!var.is_reduction) {
      get_unique_var_name(var.var_name, stream);
    }
  }
  for (auto const &rdom_var_data : rdom_vars_data) {
    get_unique_var_name(rdom_var_data.name, stream);
  }

  string clockwork_loop_name = halide_loop_name + "_loop";
  string last_loop_name = clockwork_loop_name + "_pre_rdom";
  stream << "  auto " << last_loop_name << " = prg.add_nest(";
  for (size_t i = variables.size(); i > 0; i--) {
    if (!variables[i-1].is_reduction) {
      stream << variables[i-1].var_name << ", 0, 1";
      if (i > 1) {
        stream << ", ";
      }
    }
  }
  stream << ");"<< std::endl;

  if (!rdom_vars_data.empty()) {
    string next_loop_name;
    for (size_t i = rdom_vars_data.size(); i > 0; i--) {
      RDomVarData var_data = rdom_vars_data[i-1];

      next_loop_name = clockwork_loop_name + "_" + var_data.name;
      stream << "  auto " << next_loop_name << " = " << last_loop_name
             << "->add_loop(";

      // start with a comma here and not above as always need variables, so
      // rdom variable never comes first in function call
      // also this obviates need for "," at end of each rdom set of args
      stream << var_data.name << ", " << var_data.min << ", " << var_data.max
             << ");" << std::endl;
      last_loop_name = next_loop_name;
    }
  }
  stream << "  auto " << clockwork_loop_name << " = " << last_loop_name << ";"
         << std::endl;
  return clockwork_loop_name;
}
void print_clockwork(std::ostream &mem_s, std::ostream &mem_h_s,
                     std::ostream &compute_s, Func &output_func,
                     std::string mem_name, std::string compute_name,
                     std::vector<int> bounds, int parallelism) {
  Pipeline pipeline(output_func);
  print_clockwork(mem_s, mem_h_s, compute_s, pipeline, output_func.name(),
                  mem_name, compute_name, bounds, parallelism);
}

void print_clockwork(std::ostream &mem_s, std::ostream &mem_h_s,
                     std::ostream &compute_s, Pipeline &pipeline, std::string name,
                     std::string mem_name, std::string compute_name,
                     std::vector<int> bounds, int parallelism) {
  seen_vars.clear();
  ClockworkExporter printer(mem_s, mem_h_s, compute_s);
  VariableExtractor variable_extractor{};
  RDomExtractor rdom_extractor;
  CleanVariableNames cleaner;
  
  std::string last_func_name;
  std::set<std::string> loaded_images;

  // setup the two files if this is the first time print_loop_nest called with them
  if (mem_s.tellp() == 0) {
    mem_s << "#include  \"" + mem_name + "\"" << std::endl;
    mem_s << "#include \"utils.h\"" << std::endl;
  }
  if (mem_h_s.tellp() == 0) {
    mem_h_s << "#pragma once" << std::endl;
    mem_h_s << "#include \"prog.h\"" << std::endl << std::endl;
  }
  if (compute_s.tellp() == 0) {
    compute_s << "#pragma once" << std::endl;
    compute_s << "#include  \"hw_classes.h\"" << std::endl;
  }

  // setup this function in the mem file
  mem_s << "void " << name << "_f() {" << std::endl;
  mem_h_s << "void " << name << "_f();" << std::endl;
  mem_s << "  prog prg(\"" << name << "\");" << std::endl;
  mem_s << "  prg.compute_unit_file = \"" + compute_name + "\";" << std::endl;

  IRPrinter debug_printer(std::cout);
  Parameter param = pipeline.outputs()[0].output_buffer().parameter();
  for (int i = 0; i < param.dimensions(); i++) {
    std::cout << " dimension " << i << ": ";
    Expr e = param.extent_constraint(i);
    if (e.defined() == 0) {
      std::cout << "undefined";
    }
    else {
      debug_printer.print(e);
    }
    std::cout << std::endl;
  }

  // see Lower.cpp, line 97
  // Compute an environment
  // TODO: currently the last func is the only output off the chip and the first one
  // is the input to the chip. This code only pretends to handle full pipelines.
  map<string, Function> env;
  vector<Function> output_funcs;
  for (Func f : pipeline.outputs()) {
    output_funcs.push_back(f.function());
    populate_environment(f.function(), env);
  }

  // Topologically sort the functions
  std::vector<std::string> order =
    realization_order(output_funcs, env).first;

  std::string producer_func_name, cur_func_name;
  for (int i = 0; i < (int)order.size(); i++) {
    Func f(env[order[i]]);

    for (int update_id = -1; update_id < f.num_update_definitions(); update_id++) {
      vector<Expr> args, vals;
      if (update_id == -1) {
        args.reserve(f.args().size());
        for (Var v : f.args()) {
          args.push_back(v);
        }
        vals = f.values().as_vector();
      } else {
        args = f.update_args(update_id);
        vals = f.update_values(update_id).as_vector();
      }

      // fix names
      args = cleaner.rename_exprs(args);
      vals = cleaner.rename_exprs(vals);

      /*
      map<string, Box> boxes;
      /*
      for (auto const & expr : args) {
        auto new_boxes = boxes_required(expr);
        for (auto const & kv : new_boxes) {
          if (boxes.count(kv.first) > 0) {
            auto new_box = box_union(kv.second, boxes[kv.first]);
          }
          else {
            boxes.insert(kv);
          }
        }
      }
      * /
      for (auto const & expr : vals) {
        auto new_boxes = boxes_required(expr);
        for (auto const & kv : new_boxes) {
          if (boxes.count(kv.first) > 0) {
            auto new_box = box_union(kv.second, boxes[kv.first]);
          }
          else {
            boxes.insert(kv);
          }
        }
      }
      int box_count = 0;
      for (auto const & kv : boxes) {
        std::cout << " box " << box_count++ << " (" << kv.first << "): " << kv.second << std::endl;
      }
      */ 

      // load the input image
      // TODO: handle more than 1 image in the generated clockwork code
      // I THINK THIS TODO IS HANDLED, SHOULD TEST IT THOUGH, right now
      // clockwork crashes on multiple images
      // use vals (not vals) here as an image must appear on the RHS,
      // it's a constant that isn't being assigned to
      {
        variable_extractor.extract_list(vals);
        for (auto image : variable_extractor.images) {
          // skip this image if already extracted
          if (loaded_images.count(image) == 1) {
            continue;
          }
          else {
            loaded_images.insert(image);
          }
          printer.input_img = image;
          // create a reference to the off chip image in dram
          mem_s << "  prg.add_input(\"" << image << "_offchip\");" << std::endl;
          // no rdom when loading an image
          vector<RDomVarData> empty_rdom_data = vector<RDomVarData>();
          // now create set of for mem to load the image on chip
          std::string clockwork_load_loop_name =
            create_clockwork_loop_nest(variable_extractor.variables_per_call[image],
                                       empty_rdom_data,
                                       image, mem_s);
          // once the for mem are made, put the op in them to load 
          // from off chip
          mem_s << "  auto " << image
                  << "_load_op = " << clockwork_load_loop_name
                  << "->add_op(prg.un(\"ld\"));" << std::endl;
          mem_s << "  " << image << "_load_op->add_load(\""
                  << image << "_offchip\"";
          for (auto const &var : variable_extractor.variables_per_call[image]) {
            // don't use reduction variables in loading images
            if (!var.is_reduction) {
              mem_s << ", " << var.var_name;
            }
          }
          mem_s << ");" << std::endl;
          // then put the image data in a clockwork c++ variable with the same
          // name as the name of the image in halide
          mem_s << "  " << image << "_load_op->add_store(\"" << image
                  << "_buf\", \"";
          print_indexes_clockwork(args, mem_s);
          mem_s << "\");" << std::endl;
        }
        mem_s << std::endl;
      }

      // create the loop nest for this update within this func
      // need to extract from both args and values are rdom variables may only
      // show up on right hand side of expression
      // TODO handle multiple RDOMs
      variable_extractor.extract_list(args);
      rdom_extractor.clear_state();
      for (auto &v : vals) {
        rdom_extractor.mutate(v);
      }
      printer.cur_op_name = f.name() + "_" + std::to_string(update_id + 1);
      last_func_name = f.name();
      std::string clockwork_loop_name =
        create_clockwork_loop_nest(variable_extractor.variables_per_call[NOT_A_CALL],
                                   rdom_extractor.vars_data,
                                   printer.cur_op_name, mem_s);

      // create the call to the logic-implementing C++ function
      mem_s << "  auto " << clockwork_loop_name << "_op = "
              << clockwork_loop_name << "->add_op(prg.un(\""
              << printer.cur_op_name  << "\"));" << std::endl;
      mem_s << "  " << clockwork_loop_name << "_op->add_function(\""
              << printer.cur_op_name << "_f\");" << std::endl;

      // create the the storing call but leave loading from inputs to the
      // IRVisitior when it hits the call nodes
      mem_s << "  " << clockwork_loop_name <<
        // note: intentionally storing to same location
        // for multiple update_id. Those would be rewriting same memory
        // in halide so I'm writing same buffer in clockwork
        "_op->add_store(\"" << f.name() << "_buf\"";
      std::vector<Affine_Index> indexes = AffineIndexingExtractor()
        .extract_list(args);
      mem_s << ", \"";
      for (size_t i = 0; i < indexes.size(); i++) {
        if (i != 0) {
          mem_s << ", ";
        }
        indexes[i].unquote_variable = true;
        mem_s << "floor(" << indexes[i] << ")";
      }
      mem_s << "\"";
      mem_s << "); " << std::endl;

      // create the logic-implementing C++ function
      // this code here is the wrapper for the logic implement c++ function
      // the body of the function will be created by the IRVisitor
      // NOTE: the variable extractor has to hit the calls 
      // in the same order as the printer as the mem_s must be in the
      // same order as the parameters to the function in compute_s 
      // and I think the bodies of functions must also match order of names.
      // IN SUMMARY, THERE IS SOME ORDERING USED TO CONNECT
      // functions across compute_unit_file and main file
      // that I don't understand fully
      variable_extractor.extract_list(vals);
      // clockwork passes all loads from a single buffer 
      // into the compute unit function as one parameter
      // with width equal to sum of each load widths
      // since a call in Halide is a load in clockwork,
      // need 1 parameter per call name in function parameter
      // and then split the parameter into variables for the body
      compute_s << "hw_uint<32> " << printer.cur_op_name
                  << "_f(";
      size_t compute_unit_params = 0;
      // parameters to compute function with packed loads for a single bus
      for (size_t j = 0; j < variable_extractor.calls_ordered.size(); j++) {
        if (variable_extractor.calls_ordered[j].call_count == 0) {
          std::string call_name = variable_extractor.calls_ordered[j].call_name;
          compute_s << "const hw_uint<32*"
                      << variable_extractor.times_per_call[call_name]
                      << "> &"
                      << call_name;
          if (j + 1 < variable_extractor.calls.size()) {
            compute_s << ",";
          }
          compute_unit_params++;
        }
      }
      compute_s << ") {" << std::endl;
      // unpack the bus
      for (size_t j = 0; j < variable_extractor.calls_ordered.size(); j++) {
        compute_s << "  hw_uint<32> "
                    << variable_extractor.calls_ordered[j].call_name
                    << "_"
                    << variable_extractor.calls_ordered[j].call_count
                    << " = "
                    << variable_extractor.calls_ordered[j].call_name
                    << ".extract<" 
                    << variable_extractor.calls_ordered[j].call_count * 32
                    << ","
                    << (variable_extractor.calls_ordered[j].call_count+1)*32-1
                    << ">();" << std::endl;
      }
      compute_s << "  return ";
      printer.calls_per_update.clear();
      std::cout << "debug for call name " << printer.cur_op_name << " and stage " << i << std::endl;
      for (const auto &e : vals) {
        debug_printer.print(e);
        e.accept(&printer);
      }
      std::cout << std::endl;
      compute_s << ";" << std::endl << "}" << std::endl << std::endl;
      mem_s << std::endl;
    }

  }
  // done, so infer bounds in clockwork and generate hls
  mem_s << "  prg.add_output(\"" << last_func_name << "_buf\");" << std::endl;
  mem_s << "  infer_bounds_and_unroll(\"" << last_func_name << "_buf\", {";
  for (size_t i = 0; i < bounds.size(); i++) {
    mem_s << bounds[i];
    if (i + 1 < bounds.size()) {
      mem_s << ", ";
    }
  }
  mem_s << "}, " << parallelism << ", prg);" << std::endl;
  mem_s << "  prg.pretty_print();" << std::endl;
  // this asserts false if something bad, no need to check for return value
  mem_s << "  prg.sanity_check();" << std::endl;
  mem_s << "  sanity_check_all_reads_defined(prg);" << std::endl;
  // regression_test calls the following for unoptimized code:
  // generate_unoptimized_code, then generate_regression_testbench,
  // then run_regression_tb to run unoptimized code
  // afterwards it calls the following for optimized code:
  // generate_optimized_code, then generate_regression_testbench
  // then run_regression_tb to run optimized code
  // see prog.cpp's regression_test function for this info
  //mem_s << "  regression_test(prg);" << std::endl;

  // regression test will call generate optimized code, unoptimizted code
  // , then create test benches for both and crash if not save
  // test benches (tb) are in folder where called clcowkro command, 
  // with name regression_tb_"prg_name"
  // my C++ that will be the input to the hls tool is unoptimized_prgname or optpimzed_prgname
  // I do g++ --std=c++11 regression_tb_unoptimized_brighter.cpp unoptimized_brighter.cpp 
  // to compile with testbench
  // then I run and look at regression_result_unoptimized_brighter.txt
  // to run on aws: upload soda_codes/birghter (or other prg name),
  // also scp compute unit file (like durst_compute.h) and hw_classes.h and
  // clockwork_standard_compute_units.h from clockwork root into upload folder/our_code
  // on aws, tmux, then source aws-fpga/vitis_setup.sh
  // then code to brighter folder on aws machine, source set_app.sh
  // then cd our_code in brighter, then make TARGET=hw DEVICE=$AWS_PLATFORM all
  mem_s << "  generate_optimized_code(prg);" << std::endl;
  mem_s << "  generate_regression_testbench(prg);" << std::endl;
  //mem_s << "  run_regression_tb(prg);" << std::endl;
  // check for 0, if not 0 then stop
  mem_s << "  compile_compute(prg.name + \".cpp\");" << std::endl;
  // this will leave unoptimized in place and move the optimized code
  // I need to convert xclbin to awsxclbin and then upload image myself
  mem_s << "  move_to_benchmarks_folder(prg.name);" << std::endl;
  mem_s << "}" << std::endl;
}

}  // namespace Internal
}  // namespace Halide
