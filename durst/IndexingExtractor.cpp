#include "Halide.h"
#include "IndexingExtractor.h"
#include <stdexcept>

using std::map;
using std::pair;
using std::string;
using std::vector;

namespace Halide {
namespace Internal {

void IndexingExtractor::extract_list(const std::vector<Expr> &exprs) {
  // initialize the vectors
  shifts.clear();
  downsamples.clear();
  upsamples.clear();
  for (size_t i = 0; i < exprs.size(); i++) {
    shifts.push_back(0);
    downsamples.push_back(0);
    upsamples.push_back(0);
  }
  for (size_t i = 0; i < exprs.size(); i++) {
    cur_var_index = i;
    in_shift = false;
    in_downsample = false;
    in_upsample = false;
    exprs[i].accept(this);
  }
}
/*
void ImageExtractor::visit_param(const string &ref_name, const Parameter &param) {
    if (param.defined() && param.is_buffer()) {
        const string &name = param.name();
        buffers[name] =
            BufInfo{Variable::make(type_of<halide_buffer_t *>(), name + ".buffer", param),
                        param.dimensions()};
    }
}

void ImageExtractor::visit_buffer(const string &ref_name, const Buffer<> &buffer) {
    if (buffer.defined()) {
        const string &name = buffer.name();
        buffers[name] =
            BufInfo{Variable::make(type_of<halide_buffer_t *>(), name + ".buffer", buffer),
                        buffer.dimensions()};
    }
}
*/
void IndexingExtractor::visit(const Sub *op) {
  check_simplified_error();
  in_shift = true;
  IRVisitor::visit(op);
}

void IndexingExtractor::visit(const Mul *op) {
  check_simplified_error();
  in_downsample = true;
  IRVisitor::visit(op);
}

void IndexingExtractor::visit(const Div *op) {
  check_simplified_error();
  in_upsample = true;
  IRVisitor::visit(op);
}

void IndexingExtractor::check_simplified_error() {
  if (in_shift == true || in_downsample == true || in_upsample == true) {
    throw std::runtime_error(std::string("basic indexing analysis can't handle "
                                          "unsimplified indexing expressions"));
  }
}

void IndexingExtractor::visit(const IntImm *op) {
  add_constant(op->value);
}

void IndexingExtractor::visit(const UIntImm *op) {
  add_constant(op->value);
}

void IndexingExtractor::add_constant(int constant) {
  if (in_shift) {
    shifts[cur_var_index] = constant;
  }
  else if (in_downsample) {
    downsamples[cur_var_index] = constant;
  }
  if (in_upsample) {
    upsamples[cur_var_index] = constant;
  }
}
  /*
void ImageExtractor::visit(const Variable *op) {
    visit_param(op->name, op->param);
    visit_buffer(op->name, op->image);
    symbols.insert(op->name);
}

void ImageExtractor::visit(const Load *op) {
    visit_param(op->name, op->param);
    visit_buffer(op->name, op->image);
    symbols.insert(op->name);
    IRVisitor::visit(op);
}

void ImageExtractor::visit(const Store *op) {
    visit_param(op->name, op->param);
    symbols.insert(op->name);
    IRVisitor::visit(op);
}
*/

}
}
