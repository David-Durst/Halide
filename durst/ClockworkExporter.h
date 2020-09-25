#ifndef HALIDE_INTERNAL_PRINT_LOOP_NEST_H
#define HALIDE_INTERNAL_PRINT_LOOP_NEST_H

/** \file
 *
 * Defines methods to print out the loop nest corresponding to a schedule.
 */

#include <string>
#include <vector>
#include "Halide.h"

namespace Halide {
namespace Internal {


/** Emit some clockwork code that shows the structure of the loop
 * nest specified by this pipeline's schedule, and the schedules of
 * the functions it uses. */
void print_clockwork(std::ostream &mem_s, std::ostream &mem_h_s,
                     std::ostream &compute_s, Pipeline &output_func,
                     std::string name, std::string mem_name,
                     std::string compute_name, std::vector<int> bounds,
                     int parallelism);

void print_clockwork(std::ostream &mem_s, std::ostream &mem_h_s,
                     std::ostream &compute_s, Func &output_func, 
                     std::string mem_name, std::string compute_name,
                     std::vector<int> bounds, int parallelism);

class ClockworkExporter : public IRVisitor {
public:
  explicit ClockworkExporter(std::ostream &mem_s, std::ostream &mem_h_s,
                             std::ostream &compute_s);
  virtual ~ClockworkExporter() = default;

  /** The name of the image we're reading */
  std::string input_img;

  void visit(const IntImm *) override;
  void visit(const UIntImm *) override;
  void visit(const Add *) override;
  void visit(const Sub *) override;
  void visit(const Mul *) override;
  void visit(const Div *) override;
  void visit(const Variable *op) override;
  void visit(const Call *) override;

  /** The name of the current clockwork op created by the print_loop_nest
      function and used in the call nodes to create add_load calls
      in the clockwork code
  */
  std::string cur_op_name;

  /** For the current func update, how many times has a call been used*/
  std::map<std::string, int> calls_per_update;
protected:

  Scope<Expr> constants;

  /** The stream on which we're outputting the for mem*/
  std::ostream &mem_s;

  /** The header file for stream on which we're outputting the for mem*/
  std::ostream &mem_h_s;

  /** The stream on which we're outputting the operator functions mem*/
  std::ostream &compute_s;
};

}  // namespace Internal
}  // namespace Halide

#endif
