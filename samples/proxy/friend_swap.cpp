// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from friend_swap.md.

#include <iostream>
#include <numbers>
#include <string>

#include "proxy.h"

PRO_DEF_FREE_DISPATCH(FreeToString, std::to_string, ToString);

struct Stringable : pro::facade_builder
    ::add_convention<FreeToString, std::string()>
    ::build {};

int main() {
  pro::proxy<Stringable> p0 = pro::make_proxy<Stringable>(123);
  pro::proxy<Stringable> p1 = pro::make_proxy<Stringable>(std::numbers::pi);
  std::cout << ToString(*p0) << "\n";  // Prints "10"
  std::cout << ToString(*p1) << "\n";  // Prints "3.14..."
  std::ranges::swap(p0, p1);  // finds the hidden friend
  std::cout << ToString(*p0) << "\n";  // Prints "3.14..."
  std::cout << ToString(*p1) << "\n";  // Prints "10"
}
