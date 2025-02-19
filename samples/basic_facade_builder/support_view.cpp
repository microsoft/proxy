// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from support_view.md.

#include <iostream>

#include "proxy.h"

struct RttiAware : pro::facade_builder
    ::support_rtti
    ::support_view
    ::build {};

int main() {
  pro::proxy<RttiAware> p = pro::make_proxy<RttiAware>(123);
  pro::proxy_view<RttiAware> pv = p;
  proxy_cast<int&>(*pv) = 456;  // Modifies the contained object of p
  std::cout << proxy_cast<int>(*pv) << "\n";  // Prints "456"
  std::cout << proxy_cast<int>(*p) << "\n";  // Prints "456"
}
