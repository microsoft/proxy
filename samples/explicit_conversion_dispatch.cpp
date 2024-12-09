// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from explicit_conversion_dispatch.md.

#include <iostream>

#include "proxy.h"

struct IntConvertible : pro::facade_builder
    ::add_convention<pro::conversion_dispatch, int() const>
    ::build {};

int main() {
  pro::proxy<IntConvertible> p = pro::make_proxy<IntConvertible, short>(123);  // p holds a short
  std::cout << static_cast<int>(*p) << "\n";  // Prints: "123"
}
