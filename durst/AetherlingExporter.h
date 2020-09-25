#ifndef HALIDE_SOURCE_EXPORTER_H
#define HALIDE_SOURCE_EXPORTER_H

/** \file
 * This header file defines operators that let you dump a Halide
 * expression, statement, or type directly into an output stream
 * in a human readable form.
 * E.g:
 \code
 Expr foo = ...
 std::cout << "Foo is " << foo << "\n";
 \endcode
 *
 * These operators are implemented using \ref Halide::Internal::SourceExporter
 */

#include <ostream>

#include "Halide.h"

namespace Halide {

/** Emit a halide Func in a human readable form. */
class Func;
std::ostream &export_func(std::ostream &stream, const Func &f,
                          const std::vector<int> &args_sizes);

namespace Internal {

/** An IRVisitor that emits IR to the given output stream in a human
 * readable form. Can be subclassed if you want to modify the way in
 * which it prints.
 */
class SourceExporter : public IRVisitor {
public:
    virtual ~SourceExporter() = default;

    /** Construct an SourceExporter pointed at a given output stream
     * (e.g. std::cout, or a std::ofstream) */
    explicit SourceExporter(std::ostream &s, const std::vector<int> &args_sizes);

    /** emit an expression on the output stream */
    void print(const Expr &);

    /** Emit an expression on the output stream without enclosing parens */
    void print_no_parens(const Expr &);

    /** emit a statement on the output stream */
    void print(const Stmt &);

    /** emit a comma delimited list of exprs, without any leading or
     * trailing punctuation. */
    void print_list(const std::vector<Expr> &exprs);

    /** print a const extender to match the dimensions of the func*/
    void print_nested_const();

    static void test();

    /** The name of the image we're reading */
    std::string input_img;

    /** the number of dimensions of the current func */
    int cur_func_dims;

    /** the dimensions of the dimensions done hackily */
    const std::vector<int> dims_sizes;
protected:
    Indentation get_indent() const {
        return Indentation{indent};
    }

    /** The stream on which we're outputting */
    std::ostream &stream;

    /** The current indentation level, useful for pretty-printing
     * statements */
    int indent = 0;

    /** Certain expressions do not need parens around them, e.g. the
     * args to a call are already separated by commas and a
     * surrounding set of parens. */
    bool implicit_parens = false;

    /** Either emits "(" or "", depending on the value of implicit_parens */
    void open();

    /** Either emits ")" or "", depending on the value of implicit_parens */
    void close();

    /** The symbols whose types can be inferred from values printed
     * already. */
    Scope<> known_type;

    /** A helper for printing a chain of lets with line breaks */
    void print_lets(const Let *let);

    void visit(const IntImm *) override;
    void visit(const UIntImm *) override;
    void visit(const FloatImm *) override;
    void visit(const StringImm *) override;
    void visit(const Cast *) override;
    void visit(const Variable *) override;
    void visit(const Add *) override;
    void visit(const Sub *) override;
    void visit(const Mul *) override;
    void visit(const Div *) override;
    void visit(const Mod *) override;
    void visit(const Min *) override;
    void visit(const Max *) override;
    void visit(const EQ *) override;
    void visit(const NE *) override;
    void visit(const LT *) override;
    void visit(const LE *) override;
    void visit(const GT *) override;
    void visit(const GE *) override;
    void visit(const And *) override;
    void visit(const Or *) override;
    void visit(const Not *) override;
    void visit(const Select *) override;
    void visit(const Load *) override;
    void visit(const Ramp *) override;
    void visit(const Broadcast *) override;
    void visit(const Call *) override;
    void visit(const Let *) override;
    void visit(const LetStmt *) override;
    void visit(const AssertStmt *) override;
    void visit(const ProducerConsumer *) override;
    void visit(const For *) override;
    void visit(const Acquire *) override;
    void visit(const Store *) override;
    void visit(const Provide *) override;
    void visit(const Allocate *) override;
    void visit(const Free *) override;
    void visit(const Realize *) override;
    void visit(const Block *) override;
    void visit(const Fork *) override;
    void visit(const IfThenElse *) override;
    void visit(const Evaluate *) override;
    void visit(const Shuffle *) override;
    void visit(const VectorReduce *) override;
    void visit(const Prefetch *) override;
    void visit(const Atomic *) override;
};

}  // namespace Internal
}  // namespace Halide

#endif
