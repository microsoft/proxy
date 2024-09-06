// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from support_relocation.md.

#include <memory>

#include "proxy.h"

struct Movable : pro::facade_builder::build {};

struct Trivial : pro::facade_builder
    ::support_copy<pro::constraint_level::trivial>
    ::support_relocation<pro::constraint_level::trivial>
    ::support_destruction<pro::constraint_level::trivial>
    ::build {};

int main() {
  pro::proxy<Movable> p1 = std::make_unique<int>(123);
  // pro::proxy<Trivial> p2 = std::make_unique<int>(456);  // Won't compile
  double v = 3.14;
  pro::proxy<Trivial> p3 = &v;  // Compiles because double* is trivial
}
