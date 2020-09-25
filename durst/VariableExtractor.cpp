#include "Halide.h"
#include "VariableExtractor.h"

using std::map;
using std::pair;
using std::set;
using std::string;
using std::vector;

namespace Halide {
namespace Internal {


void VariableExtractor::extract_list(const std::vector<Expr> &exprs) {
  images.clear();
  calls.clear();
  calls_ordered.clear();
  variables_in_call.clear();
  variables_per_call.clear();
  times_per_call.clear();
  for (size_t i = 0; i < exprs.size(); i++) {
    exprs[i].accept(this);
  }
}

void VariableExtractor::visit(const Variable *op) {
  // add the variable if not seen already
  if (!variables_in_call[cur_call].count(op->name)) {
    variables_in_call[cur_call].insert(op->name);
    variables_per_call[cur_call].push_back(VarInfo{op->name,
                                                   op->reduction_domain.defined()});
  }
}

void VariableExtractor::visit(const Call *op) {
  // ignore likely
  // don't change variables_per_call and cur_call
  // as just ignoring likely
  if (op->name.compare("likely") == 0) {
    IRVisitor::visit(op);
  }
  else {
    // only going to look at a particular call once, assume
    // its the same image and same variables in same order
    // otherwise
    if (!calls.count(op->name)) {
      if (op->call_type == Call::Image) {
        images.insert(op->name);
      }
      calls_ordered.push_back(CallInfo{op->name, 0});
      calls.insert(op->name);
      // call the super class visitor to search
      // for variables used in the call
      variables_in_call[op->name] = set<string>();
      variables_per_call[op->name] = vector<VarInfo>();
      std::string old_call = cur_call;
      cur_call = op->name;
      IRVisitor::visit(op);
      cur_call = old_call;
      times_per_call[op->name] = 1;
    }
    else {
      calls_ordered.push_back(CallInfo{op->name, times_per_call[op->name]});
      times_per_call[op->name]++;
    }
  }
}

}
}
