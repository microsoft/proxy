// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from make_proxy.md.

#include <iomanip>
#include <iostream>
#include <string>

#include "proxy.h"

struct Printable : pro::facade_builder
    ::add_convention<pro::operator_dispatch<"<<", true>, std::ostream&(std::ostream&) const>
    ::build {};

int main() {
  pro::proxy<Printable> p1 = pro::make_proxy<Printable>(true);  // From bool
  pro::proxy<Printable> p2 = pro::make_proxy<Printable>(123);  // From int
  pro::proxy<Printable> p3 = pro::make_proxy<Printable>(3.1415926);  // From double
  pro::proxy<Printable> p4 = pro::make_proxy<Printable>("lalala");  // From const char*
  pro::proxy<Printable> p5 = pro::make_proxy<Printable, std::string>(5, 'x');  // From a in-place constructed string
  std::cout << std::boolalpha << *p1 << "\n";  // Prints: "true"
  std::cout << *p2 << "\n";  // Prints: "123"
  std::cout << std::fixed << std::setprecision(10) << *p3 << "\n";  // Prints: "3.1415926000"
  std::cout << *p4 << "\n";  // Prints: "lalala"
  std::cout << *p5 << "\n";  // Prints: "xxxxx"
}
