// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from constructor.md.

#include <deque>
#include <iostream>
#include <vector>

#include "proxy.h"

PRO_DEF_MEM_DISPATCH(MemSize, size);
PRO_DEF_MEM_DISPATCH(MemClear, clear);

struct BasicContainer : pro::facade_builder
    ::add_convention<MemSize, std::size_t() const& noexcept>
    ::add_convention<MemClear, void() noexcept>
    ::support_copy<pro::constraint_level::nontrivial>
    ::build {};

int main() {
  std::vector<int> v{1, 2, 3};

  pro::proxy<BasicContainer> p0;
  std::cout << std::boolalpha << p0.has_value() << "\n";  // Prints "false"

  // Construct a proxy with a raw pointer
  pro::proxy<BasicContainer> p1 = &v;
  std::cout << p1.has_value() << ", " << p1->size() << "\n";  // Prints "true,3"

  // Construct a proxy with a smart pointer
  pro::proxy<BasicContainer> p2 = std::make_shared<std::deque<double>>(10);
  std::cout << p2.has_value() << ", " << p2->size() << "\n";  // Prints "true,10"

  // Copy construction
  pro::proxy<BasicContainer> p3 = p2;
  std::cout << p3.has_value() << ", " << p3->size() << "\n";  // Prints "true,10"

  // Move construction
  pro::proxy<BasicContainer> p4 = std::move(p3);
  std::cout << p4.has_value() << ", " << p4->size() << "\n";  // Prints "true,10"

  // p3 no longer contains a value
  std::cout << p3.has_value() << "\n";  // Prints "false"

  // p2 and p4 shares the same object of std::deque<double>
  p2->clear();
  std::cout << p4.has_value() << ", " << p4->size() << "\n";  // Prints "true,0"
}
