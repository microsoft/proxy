// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from support_weak.md.

#include <iostream>

#include "proxy.h"

struct Formattable : pro::facade_builder
    ::support_format
    ::support_weak
    ::build {};

int main() {
  pro::proxy<Formattable> p1 = pro::make_proxy_shared<Formattable>(123);
  pro::weak_proxy<Formattable> wp = p1;
  pro::proxy<Formattable> p2 = wp.lock();
  std::cout << std::boolalpha << p2.has_value() << "\n";  // Prints "true"
  std::cout << std::format("{}\n", *p2);  // Prints "123"

  p1.reset();
  p2.reset();
  p2 = wp.lock();
  std::cout << p2.has_value() << "\n";  // Prints "false"
}
