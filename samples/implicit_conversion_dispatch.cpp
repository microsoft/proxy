// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from implicit_conversion_dispatch.md.

#include <iomanip>
#include <iostream>

#include "proxy.h"

struct Runnable : pro::facade_builder
    ::add_convention<pro::operator_dispatch<"()">, void()>
    ::build {};

struct CopyableRunnable : pro::facade_builder
    ::support_copy<pro::constraint_level::nontrivial>
    ::add_facade<Runnable>
    ::add_direct_convention<pro::implicit_conversion_dispatch,
        pro::proxy<Runnable>() const&, pro::proxy<Runnable>() &&>
    ::build {};

int main() {
  pro::proxy<CopyableRunnable> p1 = pro::make_proxy<CopyableRunnable>(
      [] { std::cout << "Lambda expression invoked\n"; });
  auto p2 = p1;  // Copy construction
  pro::proxy<Runnable> p3 = p2;  // Implicit conversion via const reference of pro::proxy<CopyableRunnable>
  std::cout << std::boolalpha << p2.has_value() << "\n";  // Prints: "true"
  // auto p4 = p3;  // Won't compile because pro::proxy<Runnable> is not copy-constructible
  pro::proxy<Runnable> p5 = std::move(p2);  // Implicit conversion via rvalue reference of pro::proxy<CopyableRunnable>
  std::cout << p2.has_value() << "\n";  // Prints: "false"
  (*p5)();  // Prints: "Lambda expression invoked"
}
