// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from operator_dispatch.md.

#include <iomanip>
#include <iostream>
#include <numbers>

#include "proxy.h"

struct Number : pro::facade_builder
    ::add_convention<pro::operator_dispatch<"*=">, void(int)>
    ::add_convention<pro::operator_dispatch<"<<", true>, std::ostream&(std::ostream&) const&>
    ::build {};

int main() {
  pro::proxy<Number> p1 = pro::make_proxy<Number>(std::numbers::pi);
  *p1 *= 3;
  std::cout << std::setprecision(10) << *p1 << "\n";  // Prints: 9.424777961

  pro::proxy<Number> p2 = pro::make_proxy<Number>(10);
  *p2 *= 5;
  std::cout << *p2 << "\n";  // Prints: 50
}
