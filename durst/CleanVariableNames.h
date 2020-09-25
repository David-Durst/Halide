#include "Halide.h"

using std::map;
using std::pair;
using std::string;
using std::vector;

namespace Halide {
  namespace Internal {

    class CleanVariableNames : public IRMutator {
      using Internal::IRMutator::visit;
    protected:
      Expr visit(const Variable *op) override;

    public:
      std::vector<Expr> rename_exprs(const std::vector<Expr> &exprs);
    };

  }

}
