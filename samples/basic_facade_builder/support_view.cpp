// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from support_view.md.

#include <iomanip>
#include <iostream>
#include <string>

#include "proxy.h"

struct RttiAware : pro::facade_builder
    ::support_rtti
    ::add_view<RttiAware>
    ::add_view<const RttiAware>
    ::build {};

int main() {
  pro::proxy<RttiAware> p = pro::make_proxy<RttiAware>(123);
  pro::proxy_view<RttiAware> pv = p;
  pro::proxy_view<const RttiAware> pcv = p;
  proxy_cast<int&>(*pv) = 456;  // Modifies the contained object of p
  std::cout << proxy_cast<const int&>(*pcv) << "\n";  // Prints: "456"
}
