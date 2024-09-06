// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from add_convention.md.

#include <iostream>
#include <memory>
#include <string>

#include "proxy.h"

PRO_DEF_FREE_DISPATCH(FreeToString, std::to_string, ToString);

struct BasicStringable : pro::facade_builder
    ::add_convention<FreeToString, std::string() const>
    ::build {};

struct Stringable : pro::facade_builder
    ::add_facade<BasicStringable>
    ::support_copy<pro::constraint_level::nontrivial>
    ::add_direct_convention<pro::conversion_dispatch<pro::proxy<BasicStringable>>,
        pro::proxy<BasicStringable>() &&>
    ::build {};

int main() {
  pro::proxy<Stringable> p1 = std::make_shared<int>(123);
  pro::proxy<Stringable> p2 = p1;
  pro::proxy<BasicStringable> p3 = static_cast<pro::proxy<BasicStringable>>(std::move(p2));
  pro::proxy<BasicStringable> p4 = std::move(p3);
  // pro::proxy<BasicStringable> p5 = p4; // Won't compile
  std::cout << ToString(*p4) << "\n";  // Prints: "123"
  std::cout << std::boolalpha << p3.has_value() << "\n";  // Prints: "false"
}
