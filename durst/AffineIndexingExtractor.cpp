#include "Halide.h"
#include "AffineIndexingExtractor.h"
#include <stdexcept>
#include <algorithm>

using std::map;
using std::pair;
using std::string;
using std::vector;

namespace Halide {
  namespace Internal {

    /** Emit an affine indexing expression on an output stream
     * (such as std::cout) in human-readable form */
    std::ostream &operator<<(std::ostream &stream, const Affine_Index &idx) {
      // if this a quasi-affine index, handle the qausi affine part and recur
      if (idx.quasi_affine) {
        switch(idx.outer_non_affine) {
        case Max_Bound:
          stream << "max(";
          break;
        case Min_Bound:
          stream << "min(";
          break;
        default:
          stream << "invalid_non_affine_bound(";
          break;
        }
        for (size_t i = 0; i < idx.inner_affines.size(); i++) {
          if (i > 0) {
            stream << ", ";
          }
          // make sure inner affines share the unquote_variable setting
          Affine_Index inner_affine = idx.inner_affines[i];
          inner_affine.unquote_variable = idx.unquote_variable;
          stream << inner_affine;
        }
        stream << ")";
      }
      else {
        // add a starting parenthesis if going to have a denominator to divide by
        if (!idx.coeff_numerators.empty()) {
          if (idx.coeff_denominator != 1) {
            stream << "(";
          }
          for (size_t i = 0; i < idx.coeff_numerators.size(); i++) {
            // print numerator and variable with quotes
            if (idx.coeff_numerators[i] != 1) {
              stream << idx.coeff_numerators[i] << " * ";
            }

            if (idx.unquote_variable) {
              stream << "\" + ";
            }
            stream << idx.variable_names[i];
            if (idx.unquote_variable) {
              stream << " + \"";
            }

            // if not last iteration, add coefficients together
            if (i + 1 < idx.coeff_numerators.size()) {
              stream << " + ";
            }
          }
          // print constant only if not 0
          if (idx.constant < 0) {
            stream << " - " << idx.constant * -1;
          }
          else if (idx.constant > 0) {
            stream << " + " << idx.constant;
          }
          // same rules for ending parenthesis as starting
          if (idx.coeff_denominator != 1) {
            stream << ")";
          }
          if (idx.coeff_denominator != 1) {
            stream << " / " << idx.coeff_denominator;
          }
        }
        else {
          // always print constant if no variable
          stream << idx.constant;
          
        }
      }
      return stream;
    }

    Affine_Index
    AffineIndexingExtractor::compute_index(const Expr &e, const Expr &orig) {
      Affine_Index result;
      if (const Add *add = e.as<Add>()) {
        Affine_Index a = compute_index(add->a, orig);
        Affine_Index b = compute_index(add->b, orig);
        // not going to add non-affine expressions
        if (a.quasi_affine || b.quasi_affine) {
          throw_simplification_error(e, orig, "adding two quasi_affine");
        }
        // if both affine indexes have variables,
        // add the coefficients
        else if (a.has_var() && b.has_var()) {
          // first make sure same denominator for a and b
          for (auto &a_coeff: a.coeff_numerators) {
            a_coeff *= b.coeff_denominator;
          }
          a.constant *= b.coeff_denominator;
          for (auto &b_coeff: b.coeff_numerators) {
            b_coeff *= a.coeff_denominator;
          }
          b.constant *= a.coeff_denominator;
          a.coeff_denominator *= b.coeff_denominator;
          b.coeff_denominator = a.coeff_denominator;

          // then merge the variables
          result.coeff_denominator = a.coeff_denominator;
          result.variable_names = a.variable_names;
          result.coeff_numerators = a.coeff_numerators;
          result.constant = a.constant + b.constant;
          for (size_t b_index = 0; b_index < b.variable_names.size(); b_index++) {
            std::string name = b.variable_names[b_index];
            int res_index = result.var_index(name);
            // variable in b but not a, so add it to result
            if (res_index == -1) {
              result.coeff_numerators
                .push_back(b.coeff_numerators[b_index]);
              result.variable_names.push_back(name);
            }
            // variable in b and a, so add coefficients
            else {
              result.coeff_numerators[res_index] += b.coeff_numerators[b_index];
            }
          }
          return result;
        }
        // if one affine index has variables,
        // add the constants and take the initialized coefficient
        else if (a.has_var() && !b.has_var()) {
          result.coeff_numerators = a.coeff_numerators;
          result.coeff_denominator = a.coeff_denominator;
          result.constant = a.constant + (b.constant * a.coeff_denominator);
          result.variable_names = a.variable_names;
          return result;
        }
        else if (!a.has_var() && b.has_var()) {
          result.coeff_numerators = b.coeff_numerators;
          result.coeff_denominator = b.coeff_denominator;
          result.constant = (a.constant * b.coeff_denominator) + b.constant;
          result.variable_names = b.variable_names;
          return result;
        }
        // otherwise just add constants
        else {
          result.constant = a.constant + b.constant;
          return result;
        }
      }
      // TODO: DON'T DUPLICATE THIS AND ADD
      else if (const Sub *sub = e.as<Sub>()) {
        Affine_Index a = compute_index(sub->a, orig);
        Affine_Index b = compute_index(sub->b, orig);
        // not going to sub non-affine expressions
        if (a.quasi_affine || b.quasi_affine) {
          throw_simplification_error(e, orig, "subbing two quasi_affine");
        }
        // if both affine indexes have variables,
        // sub the coefficients
        else if (a.has_var() && b.has_var()) {
          // first make sure same denominator for a and b
          for (auto &a_coeff: a.coeff_numerators) {
            a_coeff *= b.coeff_denominator;
          }
          a.constant *= b.coeff_denominator;
          for (auto &b_coeff: b.coeff_numerators) {
            b_coeff *= a.coeff_denominator;
          }
          b.constant *= a.coeff_denominator;
          a.coeff_denominator *= b.coeff_denominator;
          b.coeff_denominator = a.coeff_denominator;

          // then merge the variables
          result.coeff_denominator = a.coeff_denominator;
          result.variable_names = a.variable_names;
          result.coeff_numerators = a.coeff_numerators;
          result.constant = a.constant - b.constant;
          for (size_t b_index = 0; b_index < b.variable_names.size(); b_index++) {
            std::string name = b.variable_names[b_index];
            int res_index = result.var_index(name);
            // variable in b but not a, so add it to result
            if (res_index == -1) {
              result.coeff_numerators
                .push_back(-1 * b.coeff_numerators[b_index]);
              result.variable_names.push_back(name);
            }
            // variable in b and a, so add coefficients
            else {
              result.coeff_numerators[res_index] -= b.coeff_numerators[b_index];
            }
          }
        }
        // if both affine indexes have coefficients,
        // sub the constants and take the initialized coefficient
        else if (a.has_var() && !b.has_var()) {
          result.coeff_numerators = a.coeff_numerators;
          result.coeff_denominator = a.coeff_denominator;
          result.constant = a.constant - (b.constant * a.coeff_denominator);
          result.variable_names = a.variable_names;
          return result;
        }
        else if (!a.has_var() && b.has_var()) {
          result.coeff_numerators = b.coeff_numerators;
          result.coeff_denominator = b.coeff_denominator;
          result.constant = (a.constant * b.coeff_denominator) - b.constant;
          result.variable_names = b.variable_names;
          return result;
        }
        // otherwise just sub constants
        else {
          result.constant = a.constant - b.constant;
          return result;
        }
      }
      else if (const Mul *mul = e.as<Mul>()) {
        Affine_Index a = compute_index(mul->a, orig);
        Affine_Index b = compute_index(mul->b, orig);
        // not going to mul non-affine expressions
        if (a.quasi_affine || b.quasi_affine) {
          throw_simplification_error(e, orig, "mulling two quasi_affine");
        }
        // can't multiply two affine expressions,
        // otherwise get quadratic term.
        else if (a.has_var() && b.has_var()) {
          throw_simplification_error(e, orig, "mulling two expr both with vars");
        }
        // allow multiplying by a constant
        else if (a.has_var() && !b.has_var()) {
          result.coeff_numerators = a.coeff_numerators;
          for (int &numerator : result.coeff_numerators) {
            numerator *= b.constant;
          }
          result.coeff_denominator = a.coeff_denominator;
          result.constant = a.constant * b.constant;
          result.variable_names = a.variable_names;
          return result;
        }
        else if (!a.has_var() && b.has_var()) {
          result.coeff_numerators = b.coeff_numerators;
          for (int &numerator : result.coeff_numerators) {
            numerator *= a.constant;
          }
          result.coeff_denominator = b.coeff_denominator;
          result.constant = a.constant * b.constant;
          result.variable_names = b.variable_names;
          return result;
        }
        // if both just constants, not affine expressions, can just mul them
        else if (!a.has_var() && !b.has_var()) {
          result.constant = a.constant * b.constant;
          return result;
        }
        else {
          throw_simplification_error(e, orig, "mul no match");
        }
      }
      else if (const Div *div = e.as<Div>()) {
        Affine_Index a = compute_index(div->a, orig);
        Affine_Index b = compute_index(div->b, orig);
        // not going to div non-affine expressions
        if (a.quasi_affine || b.quasi_affine) {
          throw_simplification_error(e, orig, "diving two quasi_affine");
        }
        // can technically divide two affine expressions and get an
        // affine under some situations (like (2*x)/(3*x)), but division
        // is certainly not a closed operation on affine expressions,
        // so I'm not going to handle this case for now.
        // I will handle dividing an affine expression by a constant
        // but I won't handle dividing a constant by an affine since that
        // produces x^-n which isn't affine
        if (a.has_var() && !b.has_var()) {
          result.coeff_numerators = a.coeff_numerators;
          result.coeff_denominator = a.coeff_denominator * b.constant;
          result.variable_names = a.variable_names;
          return result;
        }
        // if both just constants, not affine expressions, can just div them
        else if (!a.has_var() && !b.has_var()) {
          result.constant = a.constant / b.constant;
          return result;
        }
        else {
          throw_simplification_error(e, orig, "diving no match");
        }
      }
      else if (const IntImm *imm = e.as<IntImm>()) {
        result.constant = imm->value;
        return result;
      }
      else if (const UIntImm *uimm = e.as<UIntImm>()) {
        result.constant = uimm->value;
        return result;
      }
      else if (const Variable *var = e.as<Variable>()) {
        result.variable_names.push_back(var->name);
        result.coeff_numerators.push_back(1);
        return result;
      }
      // not actually affine stuff that we handle anyway
      else if (const Max *m = e.as<Max>()) {
        Affine_Index a = compute_index(m->a, orig);
        Affine_Index b = compute_index(m->b, orig);
        result.quasi_affine = true;
        result.outer_non_affine = Max_Bound;
        result.inner_affines.push_back(a);
        result.inner_affines.push_back(b);
        return result;
      }
      else if (const Min *m = e.as<Min>()) {
        Affine_Index a = compute_index(m->a, orig);
        Affine_Index b = compute_index(m->b, orig);
        result.quasi_affine = true;
        result.outer_non_affine = Min_Bound;
        result.inner_affines.push_back(a);
        result.inner_affines.push_back(b);
        return result;
      }
      else if (const Mod *m = e.as<Mod>()) {
        std::cerr << "mod not handled" << std::endl;
      }
      else if (const EQ *m = e.as<EQ>()) {
        std::cerr << "EQ not handled" << std::endl;
      }
      else if (const NE *m = e.as<NE>()) {
        std::cerr << "NE not handled" << std::endl;
      }
      else if (const LT *m = e.as<LT>()) {
        std::cerr << "LT not handled" << std::endl;
      }
      else if (const LE *m = e.as<LE>()) {
        std::cerr << "LE not handled" << std::endl;
      }
      else if (const GT *m = e.as<GT>()) {
        std::cerr << "GT not handled" << std::endl;
      }
      else if (const And *m = e.as<And>()) {
        std::cerr << "And not handled" << std::endl;
      }
      else if (const Or *m = e.as<Or>()) {
        std::cerr << "Or not handled" << std::endl;
      }
      else if (const Select *m = e.as<Select>()) {
        std::cerr << "Select not handled" << std::endl;
      }
      else if (const Ramp *m = e.as<Ramp>()) {
        std::cerr << "Ramp not handled" << std::endl;
      }
      else if (const Broadcast *m = e.as<Broadcast>()) {
        std::cerr << "Broadcast not handled" << std::endl;
      }
      else if (const Let *m = e.as<Let>()) {
        std::cerr << "Let not handled" << std::endl;
      }
      else if (const Cast *c = e.as<Cast>()) {
        std::cerr << "cast not handled" << std::endl;
      }
      else if (const Load *l = e.as<Load>()) {
        std::cerr << "Load not handled" << std::endl;
      }
      else if (const Shuffle *l = e.as<Shuffle>()) {
        std::cerr << "Shuffle not handled" << std::endl;
      }
      else if (const VectorReduce *l = e.as<VectorReduce>()) {
        std::cerr << "VectorReduce not handled" << std::endl;
      }
      // ignore likely nodes
      else if (const Call *c = e.as<Call>()) {
        if (c->name.compare("likely") == 0) {
          return compute_index(c->args[0], orig);
        }
        else {
          std::cerr << "what you doing in a call? that's not affine" << std::endl;
          std::cerr << "call's name: " << c->name << std::endl;
        }
      }

      // if didn't hit any of the cases, complain
      throw_simplification_error(e, orig, "generic no match");
      return result;
      
    }

    void AffineIndexingExtractor::throw_simplification_error(const Expr &cur,
                                                             const Expr &orig,
                                                             string msg) {
      IRPrinter printer(std::cerr);
      std::cerr << std::endl << msg << std::endl;
      std::cerr << std::endl << "cur expr: " << std::endl;
      printer.print(cur);
      std::cerr << std::endl << "orig expr: " << std::endl;
      printer.print(orig);
      std::cerr << std::endl;
      throw std::runtime_error("\nbasic indexing analysis can't handle"
                               "non-axis-aligned-affine indexes");
    }


    std::vector<Affine_Index>
    AffineIndexingExtractor::extract_list(const std::vector<Expr> &exprs) {
      std::vector<Affine_Index> variable_index_exprs;
      for (size_t i = 0; i < exprs.size(); i++) {
        variable_index_exprs.push_back(compute_index(exprs[i], exprs[i]));
      }
      return variable_index_exprs;
    }

  }
}
