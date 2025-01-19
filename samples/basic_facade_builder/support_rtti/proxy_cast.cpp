// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from proxy_cast.md.

#include <iostream>

#include "proxy.h"

struct RttiAware : pro::facade_builder
    ::support_rtti
    ::build {};

int main() {
  int v = 123;
  pro::proxy<RttiAware> p;
  try {
    proxy_cast<int>(*p);  // Throws
  } catch (const pro::bad_proxy_cast& e) {
    std::cout << e.what() << "\n";  // Prints an explanatory string
  }
  p = &v;
  std::cout << proxy_cast<int>(*p) << "\n";  // Prints: "123"
  proxy_cast<int&>(*p) = 456;
  std::cout << v << "\n";  // Prints: "456"
  try {
    proxy_cast<double>(*p);  // Throws
  } catch (const pro::bad_proxy_cast& e) {
    std::cout << e.what() << "\n";  // Prints an explanatory string
  }
  int* ptr1 = proxy_cast<int>(&*p);
  std::cout << std::boolalpha << ptr1 << "\n";  // Prints an address
  std::cout << std::boolalpha << &v << "\n";  // Prints the same address as above
  double* ptr2 = proxy_cast<double>(&*p);
  std::cout << ptr2 << "\n";  // Prints "0"
}
