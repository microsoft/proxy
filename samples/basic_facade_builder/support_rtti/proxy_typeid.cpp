// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from proxy_typeid.md.

#include <iostream>

#include "proxy.h"

struct RttiAware : pro::facade_builder
    ::support_rtti
    ::build {};

int main() {
  pro::proxy<RttiAware> p;
  std::cout << proxy_typeid(*p).name() << "\n";  // Prints: "v" (assuming GCC)
  p = pro::make_proxy<RttiAware>(123);
  std::cout << proxy_typeid(*p).name() << "\n";  // Prints: "i" (assuming GCC)
}
