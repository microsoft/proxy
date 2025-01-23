// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from support_format.md.

#include <format>
#include <iostream>

#include "proxy.h"

struct Formattable : pro::facade_builder
    ::support_format
    ::build {};

int main() {
  pro::proxy<Formattable> p = pro::make_proxy<Formattable>(123);
  std::cout << std::format("{}", *p) << "\n";  // Prints: "123"
  std::cout << std::format("{:*<6}", *p) << "\n";  // Prints: "123***"
}
