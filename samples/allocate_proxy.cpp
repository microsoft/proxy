// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from allocate_proxy.md.

#include <array>

#include "proxy.h"

// By default, the maximum pointer size defined by pro::facade_builder
// is 2 * sizeof(void*). This value can be overridden by `restrict_layout`.
struct Any : pro::facade_builder::build {};

int main() {
  // sizeof(std::array<int, 100>) is usually greater than 2 * sizeof(void*),
  // calling allocate_proxy has no limitation to the size and alignment of the target
  using Target = std::array<int, 100>;
  pro::proxy<Any> p1 = pro::allocate_proxy<Any, Target>(std::allocator<Target>{});
}
