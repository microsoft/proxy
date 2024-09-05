// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from conversion_dispatch.md.

#include <iomanip>
#include <iostream>

#include "proxy.h"

struct DoubleConvertible : pro::facade_builder
    ::add_convention<pro::conversion_dispatch<double>, double() const>
    ::build {};

struct Runnable : pro::facade_builder
    ::add_convention<pro::operator_dispatch<"()">, void()>
    ::build {};

struct CopyableRunnable : pro::facade_builder
    ::support_copy<pro::constraint_level::nontrivial>
    ::add_facade<Runnable>
    ::add_direct_convention<pro::conversion_dispatch<pro::proxy<Runnable>, false>,
        pro::proxy<Runnable>() const&, pro::proxy<Runnable>() &&>
    ::build {};

int main() {
  // Explicit conversion
  pro::proxy<DoubleConvertible> p1 = pro::make_proxy<DoubleConvertible>(123);  // p1 holds an integer
  std::cout << std::fixed << std::setprecision(10) << std::boolalpha;
  std::cout << static_cast<double>(*p1) << "\n";  // Prints: "123.0000000000"

  // Implicit conversion
  pro::proxy<CopyableRunnable> p2 = pro::make_proxy<CopyableRunnable>(
      [] { std::cout << "Lambda expression invoked\n"; });
  auto p3 = p2;  // Copy construction
  pro::proxy<Runnable> p4 = p3;  // Implicit conversion via const reference of pro::proxy<CopyableRunnable>
  std::cout << p3.has_value() << "\n";  // Prints: "true"
  // auto p5 = p4;  // Won't compile because pro::proxy<Runnable> is not copy-constructible
  pro::proxy<Runnable> p6 = std::move(p3);  // Implicit conversion via rvalue reference of pro::proxy<CopyableRunnable>
  std::cout << p3.has_value() << "\n";  // Prints: "false"
  (*p6)();  // Prints: "Lambda expression invoked"
}
