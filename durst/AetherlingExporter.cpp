#include <iostream>
#include <sstream>
#include "Halide.h"
#include "AetherlingExporter.h"
#include "VariableExtractor.h"
#include "IndexingExtractor.h"

namespace Halide {

using std::ostream;
using std::ostringstream;
using std::string;
using std::vector;

namespace {

void print_func(ostream &stream, const Func &func, const vector<int> &args_sizes,
                bool include_dependencies) {
    using Internal::ReductionDomain;

    Internal::SourceExporter printer(stream, args_sizes);
    Internal::VariableExtractor variable_extractor{};

    stream << func.name() << "_f ";

    // Topologically sort the functions
    std::map<std::string, Internal::Function> env =
        include_dependencies ?
            find_transitive_calls(func.function()) :
            std::map<std::string, Internal::Function>{{func.function().name(), func.function()}};
    std::vector<std::string> order =
        include_dependencies ?
            realization_order({func.function()}, env).first :
            std::vector<std::string>{func.function().name()};

    for (int i = 0; i < (int)order.size(); i++) {
        Func f(env[order[i]]);
        std::set<string> rdoms_declared;
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

            if (i == 0 && update_id == -1) {
              variable_extractor.extract_list(vals);
              for (auto image : variable_extractor.images) {
                printer.input_img = image;
                stream << image << " ";
              }
              stream << "= do " << std::endl;
            }

            class ExtractRDom : public Internal::IRMutator {
                using Internal::IRMutator::visit;

                Expr visit(const Internal::Variable *op) override {
                    if (op->reduction_domain.defined()) {
                        rdom = op->reduction_domain;
                        int idx = 0;
                        for (const auto &v : rdom.domain()) {
                            if (v.var == op->name) {
                                string new_name = rdom_name + "[" + std::to_string(idx) + "]";
                                return Internal::Variable::make(op->type, new_name);
                            }
                            idx++;
                        }
                    }
                    return op;
                }

            public:
                ReductionDomain rdom;
                string rdom_name;
            } extract_rdom;
            extract_rdom.rdom_name = "r" + std::to_string(update_id);
            for (auto &v : vals) {
                v = extract_rdom.mutate(v);
            }
            for (auto &v : args) {
                v = extract_rdom.mutate(v);
            }

            if (extract_rdom.rdom.defined()) {
                stream << "  RDom " << extract_rdom.rdom_name << "(";
                std::vector<Expr> e;
                for (const auto &d : extract_rdom.rdom.domain()) {
                    e.push_back(d.min);
                    e.push_back(d.extent);
                }
                printer.print_list(e);
                stream << ");\n";
                Expr pred = extract_rdom.rdom.predicate();
                if (pred.defined() && !is_one(pred)) {
                    pred = extract_rdom.mutate(pred);
                    stream << "  " << extract_rdom.rdom_name << ".where(";
                    printer.print_no_parens(pred);
                    stream << ");\n";
                }
            }
            stream << "  let " << f.name();// << "(";
            //printer.print_list(args);
            stream << " = ";
            printer.cur_func_dims = args.size();
            if (vals.size() > 1) {
                stream << "{";
                printer.print_list(vals);
                stream << "}";
            } else {
                printer.print_list(vals);
            }
            stream << "\n";
            if (i == (int)order.size() - 1) {
              stream << "  " << f.name() << "\n";
            }
        }
    }
    stream << func.name() << "_seq_idx = add_indexes $ seq_shallow_to_deep $ \n "
           << "  " << func.name() << "_f $ com_input_seq \"I\" (Proxy :: Proxy (";
    for (size_t i = 0; i < args_sizes.size(); i++) {
      stream << "(Seq " << args_sizes[i] << " ";
    }
    stream << "Atom_UInt8";
    for (size_t i = 0; i < args_sizes.size(); i++) {
      stream << ")";
    }
    stream << "))\n\n";
}

}  // namespace

ostream &export_func(ostream &stream, const Func &f, const vector<int> &args_sizes) {
  print_func(stream, f, args_sizes, /*include_dependencies*/ true);
  return stream;
}

namespace Internal {

SourceExporter::SourceExporter(ostream &s, const vector<int> &args_sizes)
  : dims_sizes(args_sizes), stream(s) {
    s.setf(std::ios::fixed, std::ios::floatfield);
}

void SourceExporter::print(const Expr &ir) {
    ScopedValue<bool> old(implicit_parens, false);
    ir.accept(this);
}

void SourceExporter::print_no_parens(const Expr &ir) {
    ScopedValue<bool> old(implicit_parens, true);
    ir.accept(this);
}

void SourceExporter::print(const Stmt &ir) {
    ir.accept(this);
}

void SourceExporter::print_list(const std::vector<Expr> &exprs) {
    for (size_t i = 0; i < exprs.size(); i++) {
        print_no_parens(exprs[i]);
        if (i + 1 < exprs.size()) {
            stream << ", ";
        }
    }
}

void SourceExporter::print_nested_const() {
  for (size_t i = 0; i < dims_sizes.size(); i++) {
    stream << "list_to_seq (Proxy @" << dims_sizes[i] << ") $ replicate " << dims_sizes[i] << " $ ";
  }
}

void SourceExporter::visit(const IntImm *op) {
  /*
    if (op->type == Int(32)) {
        stream << op->value;
    } else {
        stream << "(" << op->type << ")" << op->value;
    }
    */
  stream << "(const_genC (";
  print_nested_const();
  stream << "Atom_UInt8 " << op->value << ")"
         << input_img << ")";
}

void SourceExporter::visit(const UIntImm *op) {
    // stream << "(" << op->type << ")" << op->value;
  stream << "(const_genC (";
  print_nested_const();
  stream << "Atom_UInt8 " << op->value << ")"
         << input_img << ")";
}

void SourceExporter::visit(const FloatImm *op) {
    switch (op->type.bits()) {
    case 64:
        stream << op->value;
        break;
    case 32:
        stream << op->value << "f";
        break;
    case 16:
        stream << op->value << "h";
        break;
    default:
        std::cerr << "Bad bit-width for float: " << op->type << "\n";
    }
}

void SourceExporter::visit(const StringImm *op) {
    stream << "\"";
    for (size_t i = 0; i < op->value.size(); i++) {
        unsigned char c = op->value[i];
        if (c >= ' ' && c <= '~' && c != '\\' && c != '"') {
            stream << c;
        } else {
            stream << "\\";
            switch (c) {
            case '"':
                stream << "\"";
                break;
            case '\\':
                stream << "\\";
                break;
            case '\t':
                stream << "t";
                break;
            case '\r':
                stream << "r";
                break;
            case '\n':
                stream << "n";
                break;
            default:
                string hex_digits = "0123456789ABCDEF";
                stream << "x" << hex_digits[c >> 4] << hex_digits[c & 0xf];
            }
        }
    }
    stream << "\"";
}

void SourceExporter::visit(const Cast *op) {
    //stream << op->type << "(";
    print(op->value);
    //stream << ")";
}

void SourceExporter::visit(const Variable *op) {
  /*
    if (!known_type.contains(op->name) &&
        (op->type != Int(32))) {
        // Handle types already have parens
        if (op->type.is_handle()) {
            stream << op->type;
        } else {
            stream << "(" << op->type << ")";
        }
    }
    */
    stream << op->name;
}

void SourceExporter::open() {
    if (!implicit_parens) {
        stream << "(";
    }
}

void SourceExporter::close() {
    if (!implicit_parens) {
        stream << ")";
    }
}

void SourceExporter::visit(const Add *op) {
    open();
    for (int i = 0; i < cur_func_dims; i++) {
      stream << "map2C (";
    }
    stream << "\\x y -> addC $ atom_tupleC x y";
    for (int i = 0; i < cur_func_dims; i++) {
      stream << ")";
    }
    stream << " (";
    print(op->a);
    stream << ") (";
    print(op->b);
    stream << ")";
    close();
}

void SourceExporter::visit(const Sub *op) {
    open();
    print(op->a);
    stream << " - ";
    print(op->b);
    close();
}

void SourceExporter::visit(const Mul *op) {
    open();
    print(op->a);
    stream << "*";
    print(op->b);
    close();
}

void SourceExporter::visit(const Div *op) {
    open();
    print(op->a);
    stream << "/";
    print(op->b);
    close();
}

void SourceExporter::visit(const Mod *op) {
    open();
    print(op->a);
    stream << " % ";
    print(op->b);
    close();
}

void SourceExporter::visit(const Min *op) {
    stream << "min(";
    print_no_parens(op->a);
    stream << ", ";
    print_no_parens(op->b);
    stream << ")";
}

void SourceExporter::visit(const Max *op) {
    stream << "max(";
    print_no_parens(op->a);
    stream << ", ";
    print_no_parens(op->b);
    stream << ")";
}

void SourceExporter::visit(const EQ *op) {
    open();
    print(op->a);
    stream << " == ";
    print(op->b);
    close();
}

void SourceExporter::visit(const NE *op) {
    open();
    print(op->a);
    stream << " != ";
    print(op->b);
    close();
}

void SourceExporter::visit(const LT *op) {
    open();
    print(op->a);
    stream << " < ";
    print(op->b);
    close();
}

void SourceExporter::visit(const LE *op) {
    open();
    print(op->a);
    stream << " <= ";
    print(op->b);
    close();
}

void SourceExporter::visit(const GT *op) {
    open();
    print(op->a);
    stream << " > ";
    print(op->b);
    close();
}

void SourceExporter::visit(const GE *op) {
    open();
    print(op->a);
    stream << " >= ";
    print(op->b);
    close();
}

void SourceExporter::visit(const And *op) {
    open();
    print(op->a);
    stream << " && ";
    print(op->b);
    close();
}

void SourceExporter::visit(const Or *op) {
    open();
    print(op->a);
    stream << " || ";
    print(op->b);
    close();
}

void SourceExporter::visit(const Not *op) {
    stream << "!";
    print(op->a);
}

void SourceExporter::visit(const Select *op) {
    stream << "select(";
    print_no_parens(op->condition);
    stream << ", ";
    print_no_parens(op->true_value);
    stream << ", ";
    print_no_parens(op->false_value);
    stream << ")";
}

void SourceExporter::visit(const Load *op) {
    const bool has_pred = !is_one(op->predicate);
    const bool show_alignment = op->type.is_vector() && op->alignment.modulus > 1;
    if (has_pred) {
        open();
    }
    if (!known_type.contains(op->name)) {
        stream << "(" << op->type << ")";
    }
    stream << op->name << "[";
    print_no_parens(op->index);
    if (show_alignment) {
        stream << " aligned(" << op->alignment.modulus << ", " << op->alignment.remainder << ")";
    }
    stream << "]";
    if (has_pred) {
        stream << " if ";
        print(op->predicate);
        close();
    }
}

void SourceExporter::visit(const Ramp *op) {
    stream << "ramp(";
    print_no_parens(op->base);
    stream << ", ";
    print_no_parens(op->stride);
    stream << ", " << op->lanes << ")";
}

void SourceExporter::visit(const Broadcast *op) {
    stream << "x" << op->lanes << "(";
    print_no_parens(op->value);
    stream << ")";
}

void SourceExporter::visit(const Call *op) {
    // TODO: Print indication of C vs C++?
  /*
    if (!known_type.contains(op->name) &&
        (op->type != Int(32))) {
        if (op->type.is_handle()) {
            stream << op->type;  // Already has parens
        } else {
            stream << "(" << op->type << ")";
        }
    }
    */
    int old_cur_func_dims = cur_func_dims;
    cur_func_dims = 0;
    IndexingExtractor idx_extractor;
    idx_extractor.extract_list(op->args);
    for (size_t i = 0; i < op->args.size(); i++) {
      if (idx_extractor.downsamples[i] != 0) {
        for (size_t j = 0; j < i; j++) {
          stream << "mapC (";
        }
        stream << "unpartitionC . mapC (down_1dC 0) . partitionC (Proxy @" << dims_sizes[i] / idx_extractor.downsamples[i] << ")"
               << " (Proxy @" << idx_extractor.downsamples[i] << ") ";
        for (size_t j = 0; j < i; j++) {
          stream << ")";
        }
        stream << "$ ";
      }
      else if (idx_extractor.upsamples[i] != 0) {
        for (size_t j = 0; j < i; j++) {
          stream << "mapC (";
        }
        stream << "unpartitionC . mapC (up_1dC (Proxy @" << idx_extractor.upsamples[i]
               << ")) . partitionC (Proxy @" << dims_sizes[i] << ")"
               << " (Proxy @1) ";
        for (size_t j = 0; j < i; j++) {
          stream << ")";
        }
        stream << "$ ";
      }
      else if (idx_extractor.shifts[i] != 0) {
        for (size_t j = 0; j < i; j++) {
          stream << "mapC (";
        }
        stream << "shiftC (Proxy @" << idx_extractor.shifts[i] << ")";
        for (size_t j = 0; j < i; j++) {
          stream << ") ";
        }
        stream << " $ ";
      }
    }
    stream << op->name;
    cur_func_dims = old_cur_func_dims;
    /*<< "(";
                          
    print_list(op->args);
    stream << ")";
    if (op->call_type == Call::Halide &&
        op->func.defined() &&
        Function(op->func).values().size() > 1) {
        stream << "[" << std::to_string(op->value_index) << "]";
    }
    */
}

void SourceExporter::visit(const Let *op) {
    ScopedBinding<> bind(known_type, op->name);
    open();
    stream << "let " << op->name << " = ";
    print(op->value);
    stream << " in ";
    print(op->body);
    close();
}

void SourceExporter::visit(const LetStmt *op) {
    ScopedBinding<> bind(known_type, op->name);
    stream << get_indent() << "let " << op->name << " = ";
    print_no_parens(op->value);
    stream << "\n";

    print(op->body);
}

void SourceExporter::visit(const AssertStmt *op) {
    stream << get_indent() << "assert(";
    print_no_parens(op->condition);
    stream << ", ";
    print_no_parens(op->message);
    stream << ")\n";
}

void SourceExporter::visit(const ProducerConsumer *op) {
    stream << get_indent();
    if (op->is_producer) {
        stream << "produce " << op->name << " {\n";
    } else {
        stream << "consume " << op->name << " {\n";
    }
    indent++;
    print(op->body);
    indent--;
    stream << get_indent() << "}\n";
}

void SourceExporter::visit(const For *op) {
    ScopedBinding<> bind(known_type, op->name);
    stream << get_indent() << op->for_type << op->device_api << " (" << op->name << ", ";
    print_no_parens(op->min);
    stream << ", ";
    print_no_parens(op->extent);
    stream << ") {\n";

    indent++;
    print(op->body);
    indent--;

    stream << get_indent() << "}\n";
}

void SourceExporter::visit(const Acquire *op) {
    stream << get_indent() << "acquire (";
    print_no_parens(op->semaphore);
    stream << ", ";
    print_no_parens(op->count);
    stream << ") {\n";
    indent++;
    print(op->body);
    indent--;
    stream << get_indent() << "}\n";
}

void SourceExporter::print_lets(const Let *let) {
    stream << get_indent();
    ScopedBinding<> bind(known_type, let->name);
    stream << "let " << let->name << " = ";
    print_no_parens(let->value);
    stream << " in\n";
    if (const Let *next = let->body.as<Let>()) {
        print_lets(next);
    } else {
        stream << get_indent();
        print_no_parens(let->body);
        stream << "\n";
    }
}

void SourceExporter::visit(const Store *op) {
    stream << get_indent();
    const bool has_pred = !is_one(op->predicate);
    const bool show_alignment = op->value.type().is_vector() && (op->alignment.modulus > 1);
    if (has_pred) {
        stream << "predicate (";
        print_no_parens(op->predicate);
        stream << ")\n";
        indent++;
        stream << get_indent();
    }
    stream << op->name << "[";
    print_no_parens(op->index);
    if (show_alignment) {
        stream << " aligned("
               << op->alignment.modulus
               << ", "
               << op->alignment.remainder << ")";
    }
    stream << "] = ";
    if (const Let *let = op->value.as<Let>()) {
        // Use some nicer line breaks for containing Lets
        stream << "\n";
        indent += 2;
        print_lets(let);
        indent -= 2;
    } else {
        // Just print the value in-line
        print_no_parens(op->value);
    }
    stream << "\n";
    if (has_pred) {
        indent--;
    }
}

void SourceExporter::visit(const Provide *op) {
    stream << get_indent() << op->name << "(";
    print_list(op->args);
    stream << ") = ";
    if (op->values.size() > 1) {
        stream << "{";
    }
    print_list(op->values);
    if (op->values.size() > 1) {
        stream << "}";
    }

    stream << "\n";
}

void SourceExporter::visit(const Allocate *op) {
    ScopedBinding<> bind(known_type, op->name);
    stream << get_indent() << "allocate " << op->name << "[" << op->type;
    for (size_t i = 0; i < op->extents.size(); i++) {
        stream << " * ";
        print(op->extents[i]);
    }
    stream << "]";
    if (op->memory_type != MemoryType::Auto) {
        stream << " in " << op->memory_type;
    }
    if (!is_one(op->condition)) {
        stream << " if ";
        print(op->condition);
    }
    if (op->new_expr.defined()) {
        stream << "\n";
        stream << get_indent() << " custom_new { ";
        print_no_parens(op->new_expr);
        stream << " }";
    }
    if (!op->free_function.empty()) {
        stream << "\n";
        stream << get_indent() << " custom_delete { " << op->free_function << "(" << op->name << "); }";
    }
    stream << "\n";
    print(op->body);
}

void SourceExporter::visit(const Free *op) {
    stream << get_indent() << "free " << op->name;
    stream << "\n";
}

void SourceExporter::visit(const Realize *op) {
    ScopedBinding<> bind(known_type, op->name);
    stream << get_indent() << "realize " << op->name << "(";
    for (size_t i = 0; i < op->bounds.size(); i++) {
        stream << "[";
        print_no_parens(op->bounds[i].min);
        stream << ", ";
        print_no_parens(op->bounds[i].extent);
        stream << "]";
        if (i < op->bounds.size() - 1) stream << ", ";
    }
    stream << ")";
    if (op->memory_type != MemoryType::Auto) {
        stream << " in " << op->memory_type;
    }
    if (!is_one(op->condition)) {
        stream << " if ";
        print(op->condition);
    }
    stream << " {\n";

    indent++;
    print(op->body);
    indent--;

    stream << get_indent() << "}\n";
}

void SourceExporter::visit(const Prefetch *op) {
    stream << get_indent();
    const bool has_cond = !is_one(op->condition);
    if (has_cond) {
        stream << "if (";
        print_no_parens(op->condition);
        stream << ") {\n";
        indent++;
        stream << get_indent();
    }
    stream << "prefetch " << op->name << "(";
    for (size_t i = 0; i < op->bounds.size(); i++) {
        stream << "[";
        print_no_parens(op->bounds[i].min);
        stream << ", ";
        print_no_parens(op->bounds[i].extent);
        stream << "]";
        if (i < op->bounds.size() - 1) stream << ", ";
    }
    stream << ")\n";
    if (has_cond) {
        indent--;
        stream << get_indent() << "}\n";
    }
    print(op->body);
}

void SourceExporter::visit(const Block *op) {
    print(op->first);
    print(op->rest);
}

void SourceExporter::visit(const Fork *op) {
    vector<Stmt> stmts;
    stmts.push_back(op->first);
    Stmt rest = op->rest;
    while (const Fork *f = rest.as<Fork>()) {
        stmts.push_back(f->first);
        rest = f->rest;
    }
    stmts.push_back(rest);

    stream << get_indent() << "fork ";
    for (Stmt s : stmts) {
        stream << "{\n";
        indent++;
        print(s);
        indent--;
        stream << get_indent() << "} ";
    }
    stream << "\n";
}

void SourceExporter::visit(const IfThenElse *op) {
    stream << get_indent();
    while (1) {
        stream << "if (";
        print_no_parens(op->condition);
        stream << ") {\n";
        indent++;
        print(op->then_case);
        indent--;

        if (!op->else_case.defined()) {
            break;
        }

        if (const IfThenElse *nested_if = op->else_case.as<IfThenElse>()) {
            stream << get_indent() << "} else ";
            op = nested_if;
        } else {
            stream << get_indent() << "} else {\n";
            indent++;
            print(op->else_case);
            indent--;
            break;
        }
    }

    stream << get_indent() << "}\n";
}

void SourceExporter::visit(const Evaluate *op) {
    stream << get_indent();
    print_no_parens(op->value);
    stream << "\n";
}

void SourceExporter::visit(const Shuffle *op) {
    if (op->is_concat()) {
        stream << "concat_vectors(";
        print_list(op->vectors);
        stream << ")";
    } else if (op->is_interleave()) {
        stream << "interleave_vectors(";
        print_list(op->vectors);
        stream << ")";
    } else if (op->is_extract_element()) {
        stream << "extract_element(";
        print_list(op->vectors);
        stream << ", " << op->indices[0] << ")";
    } else if (op->is_slice()) {
        stream << "slice_vectors(";
        print_list(op->vectors);
        stream << ", " << op->slice_begin()
               << ", " << op->slice_stride()
               << ", " << op->indices.size()
               << ")";
    } else {
        stream << "shuffle(";
        print_list(op->vectors);
        stream << ", ";
        for (size_t i = 0; i < op->indices.size(); i++) {
            print_no_parens(op->indices[i]);
            if (i < op->indices.size() - 1) {
                stream << ", ";
            }
        }
        stream << ")";
    }
}

void SourceExporter::visit(const VectorReduce *op) {
    stream << "("
           << op->type
           << ")vector_reduce("
           << op->op
           << ", "
           << op->value
           << ")\n";
}

void SourceExporter::visit(const Atomic *op) {
    if (op->mutex_name.empty()) {
        stream << get_indent() << "atomic {\n";
    } else {
        stream << get_indent() << "atomic (";
        stream << op->mutex_name;
        stream << ") {\n";
    }
    indent += 2;
    print(op->body);
    indent -= 2;
    stream << get_indent() << "}\n";
}

}  // namespace Internal
}  // namespace Halide
