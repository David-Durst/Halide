#include "Halide.h"

using std::map;
using std::pair;
using std::set;
using std::string;
using std::vector;

namespace Halide {
namespace Internal {

struct BufInfo {
  Expr handle;
  int dimensions;
};

struct CallInfo {
  string call_name;
  // counts that this is the nth time the call has occur
  int call_count;
};

struct VarInfo {
  string var_name;
  bool is_reduction;
};

const std::string NOT_A_CALL = "";
/** Extract the calls, particularly the image calls, and variables
  * in a list of expressions. */
class VariableExtractor : public IRVisitor {
  using IRVisitor::visit;

  void visit(const Variable *op) override;
  void visit(const Call *op) override;

  /** The call the IRVisitor is currently in, used to attribute variables
   to calls. */
  string cur_call = NOT_A_CALL;

  /** Tracks which variables used at least once in a call 
      If not in a call, these will be assigned to call ""
  */
  map<string, set<string>> variables_in_call;

public:
  void extract_list(const std::vector<Expr> &exprs);

  /** Just the calls that refer to loads from images in the expressions*/
  set<string> images;
  /** All the calls that appear in the expressions*/
  set<string> calls;
  /** Ordered version of all calls in expression, with int of call
    * number */
  vector<CallInfo> calls_ordered;
  /** The variables used to access a call, in the order they appear in the call.
   If a call appears multiple times, we will assume the same variables are used
   in the same order for each call.
   If not in a call, these will be assigned to NOT_A_CALL ""
  */
  map<string, vector<VarInfo>> variables_per_call;
  /** The number of times each call appears. */
  map<string, int> times_per_call;
};
}

}
