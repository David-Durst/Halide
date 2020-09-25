#include "Halide.h"
#include "CleanVariableNames.h"

using std::map;
using std::pair;
using std::string;
using std::vector;

namespace Halide {
  namespace Internal {

    Expr CleanVariableNames::visit(const Variable *op) {
      return Variable::make(op->type, c_print_name(op->name),
                            op->image, op->param, op->reduction_domain);
    }

    std::vector<Expr>
    CleanVariableNames::rename_exprs(const std::vector<Expr> &exprs) {
      std::vector<Expr> cleaned_exprs;
      for (const auto &e : exprs) {
        cleaned_exprs.push_back(this->mutate(e));
      }
      return cleaned_exprs;
    }
  }

}
