// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from inplace_proxiable_target.md.

#include <array>

#include "proxy.h"

// By default, the maximum pointer size defined by pro::facade_builder
// is 2 * sizeof(void*). This value can be overridden by `restrict_layout`.
struct Any : pro::facade_builder::build {};

int main() {
  // sizeof(int) is usually not greater than sizeof(void*) for modern
  // 32/64-bit compilers
  static_assert(pro::inplace_proxiable_target<int, Any>);

  // sizeof(std::array<int, 100>) is usually greater than 2 * sizeof(void*)
  static_assert(!pro::inplace_proxiable_target<std::array<int, 100>, Any>);
}
