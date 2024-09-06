// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from friend_operator_equality.md.

#include <iostream>

#include "proxy.h"

struct AnyMovable : pro::facade_builder::build {};

int main() {
  pro::proxy<AnyMovable> p;
  std::cout << std::boolalpha << (p == nullptr) << "\n";  // Prints "true"
  std::cout << (p != nullptr) << "\n";  // Prints "false"
  p = std::make_unique<int>(123);
  std::cout << (p == nullptr) << "\n";  // Prints "false"
  std::cout << (p != nullptr) << "\n";  // Prints "true"
}
