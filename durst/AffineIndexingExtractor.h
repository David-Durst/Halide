#include "Halide.h"
#include <algorithm>

using std::map;
using std::pair;
using std::string;
using std::vector;

namespace Halide {
  namespace Internal {

    enum Non_Affine_Bound {Max_Bound, Min_Bound};

    /** Stores a quasi-affine expression that's an index for a variable.
      * Quasi portion is a tree structure with purely affine expressions at the root. 
      * Affine expression is stored (c_0 * x_0 + c_1 * c_1 + ... + constant) / coeff_denominator
      * This is also used as the intermediate data structure for computing an
      * affine expression from an AST.
      * During that compute, the constant can have a meaningful value even
      * if haven't found a variable yet. For example, x + 1, the call to
      * compute_index on the 1 will return an Affine_index with a constant
      * of 1, undefined as the variable name, and a constant of 1.
     */
    struct Affine_Index {
      vector<int> coeff_numerators{};
      int coeff_denominator = 1;
      int constant = 0;
      // affines around the outside of the expression
      // set quasi_affine only if this is a quasi-affine
      // expression like max surrounding a possible purely affine expression
      bool quasi_affine = false;
      Non_Affine_Bound outer_non_affine = Max_Bound;
      vector<Affine_Index> inner_affines;

      vector<string> variable_names{};
      /** When printing this, should the variables be unquoted */
      bool unquote_variable = false;

      bool has_var() { return !this->coeff_numerators.empty(); }

      /** Return the index of a name in variable_names or -1 if not present */
      int var_index(string name) {
        auto it = std::find(this->variable_names.begin(),
                            this->variable_names.end(), name);
        // return index if found
        if (it != this->variable_names.end()) {
          return std::distance(this->variable_names.begin(), it);
        }
        else {
          return -1;
        }
      }

      Affine_Index(string variable_name) : variable_names{variable_name} { }
      Affine_Index() { }
    };

    /** Emit an affine indexing expression on an output stream
     * (such as std::cout) in human-readable form */
    std::ostream &operator<<(std::ostream &stream, const Affine_Index &idx);

    class AffineIndexingExtractor {
    protected:

      Affine_Index compute_index(const Expr &e, const Expr &orig);
      void throw_simplification_error(const Expr &cur, const Expr &orig, string msg);

    public:
      std::vector<Affine_Index> extract_list(const std::vector<Expr> &exprs);
    };

  }

}
