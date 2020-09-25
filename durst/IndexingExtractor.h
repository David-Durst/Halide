#include "Halide.h"

using std::map;
using std::pair;
using std::string;
using std::vector;

namespace Halide {
  namespace Internal {

    class IndexingExtractor : public IRVisitor {
      using IRVisitor::visit;
      bool in_shift;
      bool in_downsample;
      bool in_upsample;
      size_t cur_var_index = 0;

      void visit(const Sub *op) override;
      void visit(const Mul *op) override;
      void visit(const Div *op) override;
      void visit(const IntImm *op) override;
      void visit(const UIntImm *op) override;
      void check_simplified_error();
      void add_constant(int constant);

    public:
      void extract_list(const std::vector<Expr> &exprs);
      vector<int> shifts;
      vector<int> downsamples;
      vector<int> upsamples;
    };
  }

}
