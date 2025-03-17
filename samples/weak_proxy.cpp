// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from weak_proxy.md.

#include <iostream>

#include "proxy.h"

struct Formattable : pro::facade_builder
    ::support_format
    ::build {};

int main() {
  std::shared_ptr<int> val = std::make_shared<int>(123);
  pro::weak_proxy<Formattable> wp = std::weak_ptr{val};
  pro::proxy<Formattable> p = wp.lock();
  std::cout << std::boolalpha << p.has_value() << "\n";  // Prints "true"
  std::cout << std::format("{}\n", *p);  // Prints "123"

  p.reset();
  val.reset();
  p = wp.lock();
  std::cout << p.has_value() << "\n";  // Prints "false"
}
