// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from make_proxy_shared.md.

#include <iostream>

#include "proxy.h"

struct RttiAware : pro::facade_builder
    ::support_copy<pro::constraint_level::nothrow>
    ::support_rtti
    ::support_weak
    ::build {};

int main() {
  pro::proxy<RttiAware> p1 = pro::make_proxy_shared<RttiAware>(123);
  pro::weak_proxy<RttiAware> p2 = p1;
  pro::proxy<RttiAware> p3 = p2.lock();
  std::cout << std::boolalpha << p3.has_value() << "\n";  // Prints "true"
  std::cout << proxy_cast<int>(*p3) << "\n";  // Prints "123"

  p3.reset();
  p1.reset();
  p3 = p2.lock();
  std::cout << std::boolalpha << p3.has_value() << "\n";  // Prints "false"
}
