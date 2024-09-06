// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from operator_bool.md.

#include <iostream>

#include "proxy.h"

struct AnyMovable : pro::facade_builder::build {};

int main() {
  pro::proxy<AnyMovable> p;
  std::cout << std::boolalpha << p.has_value() << "\n";  // Prints "false"
  p = pro::make_proxy<AnyMovable>(123);
  std::cout << p.has_value() << "\n";  // Prints "true"
  p = nullptr;
  std::cout << static_cast<bool>(p) << "\n";  // Prints "false"
}
