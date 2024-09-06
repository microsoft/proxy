// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from make_proxy_inplace.md.

#include <array>

#include "proxy.h"

// By default, the maximum pointer size defined by pro::facade_builder
// is 2 * sizeof(void*). This value can be overridden by `restrict_layout`.
struct Any : pro::facade_builder::build {};

int main() {
  // sizeof(int) is usually not greater than sizeof(void*) for modern
  // 32/64-bit compilers
  pro::proxy<Any> p1 = pro::make_proxy_inplace<Any>(123);

  // sizeof(std::array<int, 100>) is usually greater than 2 * sizeof(void*)
  // pro::proxy<Any> p2 = pro::make_proxy_inplace<Any, std::array<int, 100>>();  // Won't compile
}
