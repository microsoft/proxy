// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from support_copy.md.

#include <memory>

#include "proxy.h"

struct Movable : pro::facade_builder::build {};

struct Copyable : pro::facade_builder
    ::support_copy<pro::constraint_level::nontrivial>
    ::build {};

int main() {
  pro::proxy<Movable> p1 = std::make_unique<int>(123);
  // pro::proxy<Copyable> p2 = std::make_unique<int>(123);  // Won't compile
  pro::proxy<Copyable> p3 = std::make_shared<int>(456);
  // auto p4 = p1;  // Won't compile
  auto p5 = p3;
}
