// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from support_rtti.md.

#include <iostream>

#include "proxy.h"

struct RttiAware : pro::facade_builder
    ::support_rtti
    ::support_direct_rtti
    ::build {};

int main() {
  int v = 123;
  pro::proxy<RttiAware> p = &v;
  std::cout << proxy_typeid(p).name() << "\n";  // Prints: "Pi" (assuming GCC)
  std::cout << proxy_cast<int*>(p) << "\n";  // Prints the address of v
  std::cout << proxy_typeid(*p).name() << "\n";  // Prints: "i" (assuming GCC)
  std::cout << proxy_cast<int>(*p) << "\n";  // Prints: "123"
}
