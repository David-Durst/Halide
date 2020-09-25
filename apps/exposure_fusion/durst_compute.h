#pragma once
#include  "hw_classes.h"
hw_uint<32> input_im_0_f(const hw_uint<32*1> &input) {
  hw_uint<32> input_0 = input.extract<0,31>();
  return input_0;
}

hw_uint<32> lambda_0_0_f(const hw_uint<32*1> &input_im) {
  hw_uint<32> input_im_0 = input_im.extract<0,31>();
  return input_im_0;
}

hw_uint<32> repeat_edge_0_f(const hw_uint<32*1> &lambda_0) {
  hw_uint<32> lambda_0_0 = lambda_0.extract<0,31>();
  return lambda_0_0;
}

hw_uint<32> f0_0_f(const hw_uint<32*1> &repeat_edge) {
  hw_uint<32> repeat_edge_0 = repeat_edge.extract<0,31>();
  return repeat_edge_0;
}

hw_uint<32> f1_0_f(const hw_uint<32*1> &repeat_edge) {
  hw_uint<32> repeat_edge_0 = repeat_edge.extract<0,31>();
  return repeat_edge_0 * 3;
}

hw_uint<32> f3_0_f(const hw_uint<32*1> &f1) {
  hw_uint<32> f1_0 = f1.extract<0,31>();
  return 2 * f1_0;
}

hw_uint<32> f2_0_f(const hw_uint<32*1> &f0) {
  hw_uint<32> f0_0 = f0.extract<0,31>();
  return 2 * f0_0;
}

hw_uint<32> f4_0_f(const hw_uint<32*1> &f3,const hw_uint<32*1> &f2) {
  hw_uint<32> f3_0 = f3.extract<0,31>();
  hw_uint<32> f2_0 = f2.extract<0,31>();
  return f3_0 + f2_0;
}

hw_uint<32> f6_0_f(const hw_uint<32*1> &f3,const hw_uint<32*1> &f4) {
  hw_uint<32> f3_0 = f3.extract<0,31>();
  hw_uint<32> f4_0 = f4.extract<0,31>();
  return 256 * f3_0 / f4_0;
}

hw_uint<32> gaussian_pyramid_0_0_f(const hw_uint<32*1> &f1,const hw_uint<32*1> &f0,const hw_uint<32*1> &f6) {
  hw_uint<32> f1_0 = f1.extract<0,31>();
  hw_uint<32> f0_0 = f0.extract<0,31>();
  hw_uint<32> f6_0 = f6.extract<0,31>();
  return _c0f1_0_c1f0_0f6_0;
}

hw_uint<32> gaussian_pyramid_1_0_f() {
  return 0;
}

hw_uint<32> gaussian_pyramid_1_1_f(const hw_uint<32*1> &gaussian_pyramid_1,const hw_uint<32*1> &gaussian_pyramid_0) {
  hw_uint<32> gaussian_pyramid_1_0 = gaussian_pyramid_1.extract<0,31>();
  hw_uint<32> gaussian_pyramid_0_0 = gaussian_pyramid_0.extract<0,31>();
  return gaussian_pyramid_1_0 + gaussian_pyramid_0_0;
}

hw_uint<32> gaussian_pyramid_1_2_f(const hw_uint<32*1> &gaussian_pyramid_1) {
  hw_uint<32> gaussian_pyramid_1_0 = gaussian_pyramid_1.extract<0,31>();
  return gaussian_pyramid_1_0 / 9;
}

hw_uint<32> gaussian_pyramid_2_0_f() {
  return 0;
}

hw_uint<32> gaussian_pyramid_2_1_f(const hw_uint<32*1> &gaussian_pyramid_2,const hw_uint<32*1> &gaussian_pyramid_1) {
  hw_uint<32> gaussian_pyramid_2_0 = gaussian_pyramid_2.extract<0,31>();
  hw_uint<32> gaussian_pyramid_1_0 = gaussian_pyramid_1.extract<0,31>();
  return gaussian_pyramid_2_0 + gaussian_pyramid_1_0;
}

hw_uint<32> gaussian_pyramid_2_2_f(const hw_uint<32*1> &gaussian_pyramid_2) {
  hw_uint<32> gaussian_pyramid_2_0 = gaussian_pyramid_2.extract<0,31>();
  return gaussian_pyramid_2_0 / 9;
}

hw_uint<32> gaussian_pyramid_3_0_f() {
  return 0;
}

hw_uint<32> gaussian_pyramid_3_1_f(const hw_uint<32*1> &gaussian_pyramid_3,const hw_uint<32*1> &gaussian_pyramid_2) {
  hw_uint<32> gaussian_pyramid_3_0 = gaussian_pyramid_3.extract<0,31>();
  hw_uint<32> gaussian_pyramid_2_0 = gaussian_pyramid_2.extract<0,31>();
  return gaussian_pyramid_3_0 + gaussian_pyramid_2_0;
}

hw_uint<32> gaussian_pyramid_3_2_f(const hw_uint<32*1> &gaussian_pyramid_3) {
  hw_uint<32> gaussian_pyramid_3_0 = gaussian_pyramid_3.extract<0,31>();
  return gaussian_pyramid_3_0 / 9;
}

hw_uint<32> gaussian_pyramid_4_0_f() {
  return 0;
}

hw_uint<32> gaussian_pyramid_4_1_f(const hw_uint<32*1> &gaussian_pyramid_4,const hw_uint<32*1> &gaussian_pyramid_3) {
  hw_uint<32> gaussian_pyramid_4_0 = gaussian_pyramid_4.extract<0,31>();
  hw_uint<32> gaussian_pyramid_3_0 = gaussian_pyramid_3.extract<0,31>();
  return gaussian_pyramid_4_0 + gaussian_pyramid_3_0;
}

hw_uint<32> gaussian_pyramid_4_2_f(const hw_uint<32*1> &gaussian_pyramid_4) {
  hw_uint<32> gaussian_pyramid_4_0 = gaussian_pyramid_4.extract<0,31>();
  return gaussian_pyramid_4_0 / 9;
}

hw_uint<32> gaussian_pyramid_5_0_f() {
  return 0;
}

hw_uint<32> gaussian_pyramid_5_1_f(const hw_uint<32*1> &gaussian_pyramid_5,const hw_uint<32*1> &gaussian_pyramid_4) {
  hw_uint<32> gaussian_pyramid_5_0 = gaussian_pyramid_5.extract<0,31>();
  hw_uint<32> gaussian_pyramid_4_0 = gaussian_pyramid_4.extract<0,31>();
  return gaussian_pyramid_5_0 + gaussian_pyramid_4_0;
}

hw_uint<32> gaussian_pyramid_5_2_f(const hw_uint<32*1> &gaussian_pyramid_5) {
  hw_uint<32> gaussian_pyramid_5_0 = gaussian_pyramid_5.extract<0,31>();
  return gaussian_pyramid_5_0 / 9;
}

hw_uint<32> gaussian_pyramid_6_0_f() {
  return 0;
}

hw_uint<32> gaussian_pyramid_6_1_f(const hw_uint<32*1> &gaussian_pyramid_6,const hw_uint<32*1> &gaussian_pyramid_5) {
  hw_uint<32> gaussian_pyramid_6_0 = gaussian_pyramid_6.extract<0,31>();
  hw_uint<32> gaussian_pyramid_5_0 = gaussian_pyramid_5.extract<0,31>();
  return gaussian_pyramid_6_0 + gaussian_pyramid_5_0;
}

hw_uint<32> gaussian_pyramid_6_2_f(const hw_uint<32*1> &gaussian_pyramid_6) {
  hw_uint<32> gaussian_pyramid_6_0 = gaussian_pyramid_6.extract<0,31>();
  return gaussian_pyramid_6_0 / 9;
}

hw_uint<32> gaussian_pyramid_7_0_f() {
  return 0;
}

hw_uint<32> gaussian_pyramid_7_1_f(const hw_uint<32*1> &gaussian_pyramid_7,const hw_uint<32*1> &gaussian_pyramid_6) {
  hw_uint<32> gaussian_pyramid_7_0 = gaussian_pyramid_7.extract<0,31>();
  hw_uint<32> gaussian_pyramid_6_0 = gaussian_pyramid_6.extract<0,31>();
  return gaussian_pyramid_7_0 + gaussian_pyramid_6_0;
}

hw_uint<32> gaussian_pyramid_7_2_f(const hw_uint<32*1> &gaussian_pyramid_7) {
  hw_uint<32> gaussian_pyramid_7_0 = gaussian_pyramid_7.extract<0,31>();
  return gaussian_pyramid_7_0 / 9;
}

hw_uint<32> gaussian_pyramid_8_0_f() {
  return 0;
}

hw_uint<32> gaussian_pyramid_8_1_f(const hw_uint<32*1> &gaussian_pyramid_8,const hw_uint<32*1> &gaussian_pyramid_7) {
  hw_uint<32> gaussian_pyramid_8_0 = gaussian_pyramid_8.extract<0,31>();
  hw_uint<32> gaussian_pyramid_7_0 = gaussian_pyramid_7.extract<0,31>();
  return gaussian_pyramid_8_0 + gaussian_pyramid_7_0;
}

hw_uint<32> gaussian_pyramid_8_2_f(const hw_uint<32*1> &gaussian_pyramid_8) {
  hw_uint<32> gaussian_pyramid_8_0 = gaussian_pyramid_8.extract<0,31>();
  return gaussian_pyramid_8_0 / 9;
}

hw_uint<32> gaussian_pyramid_9_0_f() {
  return 0;
}

hw_uint<32> gaussian_pyramid_9_1_f(const hw_uint<32*1> &gaussian_pyramid_9,const hw_uint<32*1> &gaussian_pyramid_8) {
  hw_uint<32> gaussian_pyramid_9_0 = gaussian_pyramid_9.extract<0,31>();
  hw_uint<32> gaussian_pyramid_8_0 = gaussian_pyramid_8.extract<0,31>();
  return gaussian_pyramid_9_0 + gaussian_pyramid_8_0;
}

hw_uint<32> gaussian_pyramid_9_2_f(const hw_uint<32*1> &gaussian_pyramid_9) {
  hw_uint<32> gaussian_pyramid_9_0 = gaussian_pyramid_9.extract<0,31>();
  return gaussian_pyramid_9_0 / 9;
}

hw_uint<32> laplacian_pyramid_9_0_f(const hw_uint<32*1> &gaussian_pyramid_9) {
  hw_uint<32> gaussian_pyramid_9_0 = gaussian_pyramid_9.extract<0,31>();
  return gaussian_pyramid_9_0;
}

hw_uint<32> merged_pyramid_9_0_f(const hw_uint<32*1> &gaussian_pyramid_9,const hw_uint<32*2> &laplacian_pyramid_9) {
  hw_uint<32> gaussian_pyramid_9_0 = gaussian_pyramid_9.extract<0,31>();
  hw_uint<32> laplacian_pyramid_9_0 = laplacian_pyramid_9.extract<0,31>();
  hw_uint<32> laplacian_pyramid_9_1 = laplacian_pyramid_9.extract<32,63>();
  return gaussian_pyramid_9_0laplacian_pyramid_9_0 * _t45 + laplacian_pyramid_9_1 * 256 - _t45 / 256;
}

hw_uint<32> collapsed_pyramid_9_0_f(const hw_uint<32*1> &merged_pyramid_9) {
  hw_uint<32> merged_pyramid_9_0 = merged_pyramid_9.extract<0,31>();
  return merged_pyramid_9_0;
}

hw_uint<32> laplacian_pyramid_8_0_f(const hw_uint<32*1> &gaussian_pyramid_8,const hw_uint<32*1> &gaussian_pyramid_9) {
  hw_uint<32> gaussian_pyramid_8_0 = gaussian_pyramid_8.extract<0,31>();
  hw_uint<32> gaussian_pyramid_9_0 = gaussian_pyramid_9.extract<0,31>();
  return gaussian_pyramid_8_0 - gaussian_pyramid_9_0;
}

hw_uint<32> merged_pyramid_8_0_f(const hw_uint<32*1> &gaussian_pyramid_8,const hw_uint<32*2> &laplacian_pyramid_8) {
  hw_uint<32> gaussian_pyramid_8_0 = gaussian_pyramid_8.extract<0,31>();
  hw_uint<32> laplacian_pyramid_8_0 = laplacian_pyramid_8.extract<0,31>();
  hw_uint<32> laplacian_pyramid_8_1 = laplacian_pyramid_8.extract<32,63>();
  return gaussian_pyramid_8_0laplacian_pyramid_8_0 * _t44 + laplacian_pyramid_8_1 * 256 - _t44 / 256;
}

hw_uint<32> collapsed_pyramid_8_0_f(const hw_uint<32*1> &collapsed_pyramid_9,const hw_uint<32*1> &merged_pyramid_8) {
  hw_uint<32> collapsed_pyramid_9_0 = collapsed_pyramid_9.extract<0,31>();
  hw_uint<32> merged_pyramid_8_0 = merged_pyramid_8.extract<0,31>();
  return collapsed_pyramid_9_0 + merged_pyramid_8_0;
}

hw_uint<32> laplacian_pyramid_7_0_f(const hw_uint<32*1> &gaussian_pyramid_7,const hw_uint<32*1> &gaussian_pyramid_8) {
  hw_uint<32> gaussian_pyramid_7_0 = gaussian_pyramid_7.extract<0,31>();
  hw_uint<32> gaussian_pyramid_8_0 = gaussian_pyramid_8.extract<0,31>();
  return gaussian_pyramid_7_0 - gaussian_pyramid_8_0;
}

hw_uint<32> merged_pyramid_7_0_f(const hw_uint<32*1> &gaussian_pyramid_7,const hw_uint<32*2> &laplacian_pyramid_7) {
  hw_uint<32> gaussian_pyramid_7_0 = gaussian_pyramid_7.extract<0,31>();
  hw_uint<32> laplacian_pyramid_7_0 = laplacian_pyramid_7.extract<0,31>();
  hw_uint<32> laplacian_pyramid_7_1 = laplacian_pyramid_7.extract<32,63>();
  return gaussian_pyramid_7_0laplacian_pyramid_7_0 * _t43 + laplacian_pyramid_7_1 * 256 - _t43 / 256;
}

hw_uint<32> collapsed_pyramid_7_0_f(const hw_uint<32*1> &collapsed_pyramid_8,const hw_uint<32*1> &merged_pyramid_7) {
  hw_uint<32> collapsed_pyramid_8_0 = collapsed_pyramid_8.extract<0,31>();
  hw_uint<32> merged_pyramid_7_0 = merged_pyramid_7.extract<0,31>();
  return collapsed_pyramid_8_0 + merged_pyramid_7_0;
}

hw_uint<32> laplacian_pyramid_6_0_f(const hw_uint<32*1> &gaussian_pyramid_6,const hw_uint<32*1> &gaussian_pyramid_7) {
  hw_uint<32> gaussian_pyramid_6_0 = gaussian_pyramid_6.extract<0,31>();
  hw_uint<32> gaussian_pyramid_7_0 = gaussian_pyramid_7.extract<0,31>();
  return gaussian_pyramid_6_0 - gaussian_pyramid_7_0;
}

hw_uint<32> merged_pyramid_6_0_f(const hw_uint<32*1> &gaussian_pyramid_6,const hw_uint<32*2> &laplacian_pyramid_6) {
  hw_uint<32> gaussian_pyramid_6_0 = gaussian_pyramid_6.extract<0,31>();
  hw_uint<32> laplacian_pyramid_6_0 = laplacian_pyramid_6.extract<0,31>();
  hw_uint<32> laplacian_pyramid_6_1 = laplacian_pyramid_6.extract<32,63>();
  return gaussian_pyramid_6_0laplacian_pyramid_6_0 * _t42 + laplacian_pyramid_6_1 * 256 - _t42 / 256;
}

hw_uint<32> collapsed_pyramid_6_0_f(const hw_uint<32*1> &collapsed_pyramid_7,const hw_uint<32*1> &merged_pyramid_6) {
  hw_uint<32> collapsed_pyramid_7_0 = collapsed_pyramid_7.extract<0,31>();
  hw_uint<32> merged_pyramid_6_0 = merged_pyramid_6.extract<0,31>();
  return collapsed_pyramid_7_0 + merged_pyramid_6_0;
}

hw_uint<32> laplacian_pyramid_5_0_f(const hw_uint<32*1> &gaussian_pyramid_5,const hw_uint<32*1> &gaussian_pyramid_6) {
  hw_uint<32> gaussian_pyramid_5_0 = gaussian_pyramid_5.extract<0,31>();
  hw_uint<32> gaussian_pyramid_6_0 = gaussian_pyramid_6.extract<0,31>();
  return gaussian_pyramid_5_0 - gaussian_pyramid_6_0;
}

hw_uint<32> merged_pyramid_5_0_f(const hw_uint<32*1> &gaussian_pyramid_5,const hw_uint<32*2> &laplacian_pyramid_5) {
  hw_uint<32> gaussian_pyramid_5_0 = gaussian_pyramid_5.extract<0,31>();
  hw_uint<32> laplacian_pyramid_5_0 = laplacian_pyramid_5.extract<0,31>();
  hw_uint<32> laplacian_pyramid_5_1 = laplacian_pyramid_5.extract<32,63>();
  return gaussian_pyramid_5_0laplacian_pyramid_5_0 * _t41 + laplacian_pyramid_5_1 * 256 - _t41 / 256;
}

hw_uint<32> collapsed_pyramid_5_0_f(const hw_uint<32*1> &collapsed_pyramid_6,const hw_uint<32*1> &merged_pyramid_5) {
  hw_uint<32> collapsed_pyramid_6_0 = collapsed_pyramid_6.extract<0,31>();
  hw_uint<32> merged_pyramid_5_0 = merged_pyramid_5.extract<0,31>();
  return collapsed_pyramid_6_0 + merged_pyramid_5_0;
}

hw_uint<32> laplacian_pyramid_4_0_f(const hw_uint<32*1> &gaussian_pyramid_4,const hw_uint<32*1> &gaussian_pyramid_5) {
  hw_uint<32> gaussian_pyramid_4_0 = gaussian_pyramid_4.extract<0,31>();
  hw_uint<32> gaussian_pyramid_5_0 = gaussian_pyramid_5.extract<0,31>();
  return gaussian_pyramid_4_0 - gaussian_pyramid_5_0;
}

hw_uint<32> merged_pyramid_4_0_f(const hw_uint<32*1> &gaussian_pyramid_4,const hw_uint<32*2> &laplacian_pyramid_4) {
  hw_uint<32> gaussian_pyramid_4_0 = gaussian_pyramid_4.extract<0,31>();
  hw_uint<32> laplacian_pyramid_4_0 = laplacian_pyramid_4.extract<0,31>();
  hw_uint<32> laplacian_pyramid_4_1 = laplacian_pyramid_4.extract<32,63>();
  return gaussian_pyramid_4_0laplacian_pyramid_4_0 * _t40 + laplacian_pyramid_4_1 * 256 - _t40 / 256;
}

hw_uint<32> collapsed_pyramid_4_0_f(const hw_uint<32*1> &collapsed_pyramid_5,const hw_uint<32*1> &merged_pyramid_4) {
  hw_uint<32> collapsed_pyramid_5_0 = collapsed_pyramid_5.extract<0,31>();
  hw_uint<32> merged_pyramid_4_0 = merged_pyramid_4.extract<0,31>();
  return collapsed_pyramid_5_0 + merged_pyramid_4_0;
}

hw_uint<32> laplacian_pyramid_3_0_f(const hw_uint<32*1> &gaussian_pyramid_3,const hw_uint<32*1> &gaussian_pyramid_4) {
  hw_uint<32> gaussian_pyramid_3_0 = gaussian_pyramid_3.extract<0,31>();
  hw_uint<32> gaussian_pyramid_4_0 = gaussian_pyramid_4.extract<0,31>();
  return gaussian_pyramid_3_0 - gaussian_pyramid_4_0;
}

hw_uint<32> merged_pyramid_3_0_f(const hw_uint<32*1> &gaussian_pyramid_3,const hw_uint<32*2> &laplacian_pyramid_3) {
  hw_uint<32> gaussian_pyramid_3_0 = gaussian_pyramid_3.extract<0,31>();
  hw_uint<32> laplacian_pyramid_3_0 = laplacian_pyramid_3.extract<0,31>();
  hw_uint<32> laplacian_pyramid_3_1 = laplacian_pyramid_3.extract<32,63>();
  return gaussian_pyramid_3_0laplacian_pyramid_3_0 * _t39 + laplacian_pyramid_3_1 * 256 - _t39 / 256;
}

hw_uint<32> collapsed_pyramid_3_0_f(const hw_uint<32*1> &collapsed_pyramid_4,const hw_uint<32*1> &merged_pyramid_3) {
  hw_uint<32> collapsed_pyramid_4_0 = collapsed_pyramid_4.extract<0,31>();
  hw_uint<32> merged_pyramid_3_0 = merged_pyramid_3.extract<0,31>();
  return collapsed_pyramid_4_0 + merged_pyramid_3_0;
}

hw_uint<32> laplacian_pyramid_2_0_f(const hw_uint<32*1> &gaussian_pyramid_2,const hw_uint<32*1> &gaussian_pyramid_3) {
  hw_uint<32> gaussian_pyramid_2_0 = gaussian_pyramid_2.extract<0,31>();
  hw_uint<32> gaussian_pyramid_3_0 = gaussian_pyramid_3.extract<0,31>();
  return gaussian_pyramid_2_0 - gaussian_pyramid_3_0;
}

hw_uint<32> merged_pyramid_2_0_f(const hw_uint<32*1> &gaussian_pyramid_2,const hw_uint<32*2> &laplacian_pyramid_2) {
  hw_uint<32> gaussian_pyramid_2_0 = gaussian_pyramid_2.extract<0,31>();
  hw_uint<32> laplacian_pyramid_2_0 = laplacian_pyramid_2.extract<0,31>();
  hw_uint<32> laplacian_pyramid_2_1 = laplacian_pyramid_2.extract<32,63>();
  return gaussian_pyramid_2_0laplacian_pyramid_2_0 * _t38 + laplacian_pyramid_2_1 * 256 - _t38 / 256;
}

hw_uint<32> collapsed_pyramid_2_0_f(const hw_uint<32*1> &collapsed_pyramid_3,const hw_uint<32*1> &merged_pyramid_2) {
  hw_uint<32> collapsed_pyramid_3_0 = collapsed_pyramid_3.extract<0,31>();
  hw_uint<32> merged_pyramid_2_0 = merged_pyramid_2.extract<0,31>();
  return collapsed_pyramid_3_0 + merged_pyramid_2_0;
}

hw_uint<32> laplacian_pyramid_1_0_f(const hw_uint<32*1> &gaussian_pyramid_1,const hw_uint<32*1> &gaussian_pyramid_2) {
  hw_uint<32> gaussian_pyramid_1_0 = gaussian_pyramid_1.extract<0,31>();
  hw_uint<32> gaussian_pyramid_2_0 = gaussian_pyramid_2.extract<0,31>();
  return gaussian_pyramid_1_0 - gaussian_pyramid_2_0;
}

hw_uint<32> merged_pyramid_1_0_f(const hw_uint<32*1> &gaussian_pyramid_1,const hw_uint<32*2> &laplacian_pyramid_1) {
  hw_uint<32> gaussian_pyramid_1_0 = gaussian_pyramid_1.extract<0,31>();
  hw_uint<32> laplacian_pyramid_1_0 = laplacian_pyramid_1.extract<0,31>();
  hw_uint<32> laplacian_pyramid_1_1 = laplacian_pyramid_1.extract<32,63>();
  return gaussian_pyramid_1_0laplacian_pyramid_1_0 * _t37 + laplacian_pyramid_1_1 * 256 - _t37 / 256;
}

hw_uint<32> collapsed_pyramid_1_0_f(const hw_uint<32*1> &collapsed_pyramid_2,const hw_uint<32*1> &merged_pyramid_1) {
  hw_uint<32> collapsed_pyramid_2_0 = collapsed_pyramid_2.extract<0,31>();
  hw_uint<32> merged_pyramid_1_0 = merged_pyramid_1.extract<0,31>();
  return collapsed_pyramid_2_0 + merged_pyramid_1_0;
}

hw_uint<32> laplacian_pyramid_0_0_f(const hw_uint<32*1> &gaussian_pyramid_0,const hw_uint<32*1> &gaussian_pyramid_1) {
  hw_uint<32> gaussian_pyramid_0_0 = gaussian_pyramid_0.extract<0,31>();
  hw_uint<32> gaussian_pyramid_1_0 = gaussian_pyramid_1.extract<0,31>();
  return gaussian_pyramid_0_0 - gaussian_pyramid_1_0;
}

hw_uint<32> merged_pyramid_0_0_f(const hw_uint<32*1> &gaussian_pyramid_0,const hw_uint<32*2> &laplacian_pyramid_0) {
  hw_uint<32> gaussian_pyramid_0_0 = gaussian_pyramid_0.extract<0,31>();
  hw_uint<32> laplacian_pyramid_0_0 = laplacian_pyramid_0.extract<0,31>();
  hw_uint<32> laplacian_pyramid_0_1 = laplacian_pyramid_0.extract<32,63>();
  return gaussian_pyramid_0_0laplacian_pyramid_0_0 * _t36 + laplacian_pyramid_0_1 * 256 - _t36 / 256;
}

hw_uint<32> collapsed_pyramid_0_0_f(const hw_uint<32*1> &collapsed_pyramid_1,const hw_uint<32*1> &merged_pyramid_0) {
  hw_uint<32> collapsed_pyramid_1_0 = collapsed_pyramid_1.extract<0,31>();
  hw_uint<32> merged_pyramid_0_0 = merged_pyramid_0.extract<0,31>();
  return collapsed_pyramid_1_0 + merged_pyramid_0_0;
}

