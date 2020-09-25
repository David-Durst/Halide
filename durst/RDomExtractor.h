#include "Halide.h"
#include <sstream>
using std::string;
using std::vector;
using std::stringstream;

namespace Halide {
namespace Internal {

struct RDomVarData {
  string name;
  // the mins and maxs exprs as strings for running at clockwork compile time
  // clockwork prog add_nest uses min/max rather than min/extent
  string min;
  string max;
};

class RDomExtractor : public IRMutator {
  using IRMutator::visit;

  Expr visit(const Internal::Variable *op) override {
    // only look at RDom once
    if (!found_rdom && op->reduction_domain.defined()) {
      found_rdom = true;
      rdom = op->reduction_domain;
      for (const auto &v : rdom.domain()) {
        RDomVarData var_data;
        // I've cleaner variable names, but not rdom variables
        var_data.name = c_print_name(v.var);

        // get min and extent
        std::stringstream min_strm, extent_strm;
        IRPrinter min_printer(min_strm), extent_printer(extent_strm);
        min_printer.print(v.min);
        extent_printer.print(v.extent);

        // save vars as mins and maxs
        string min_str = min_strm.str();
        var_data.min = min_str;
        // subtract by 1 at end because max = min + extent - 1
        var_data.max = min_str + " + (" + extent_strm.str() + ") - 1";
        vars_data.push_back(var_data);
      }
    }
    return op;
  }

public:
  ReductionDomain rdom;
  bool found_rdom = false;
  vector<RDomVarData> vars_data;
  void clear_state() {
    found_rdom = false;
    vars_data.empty();
  }
};

}
}
