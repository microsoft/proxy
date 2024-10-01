// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from proxy_invoke.md.

#include <iostream>
#include <string>

#include "proxy.h"

PRO_DEF_FREE_DISPATCH(FreeToString, std::to_string, ToString);

struct Stringable : pro::facade_builder
    ::add_convention<FreeToString, std::string() const>
    ::build {};

int main() {
  int a = 123;
  pro::proxy<Stringable> p = &a;
  std::cout << ToString(*p) << "\n";  // Invokes with accessor, prints: "123"

  using C = std::tuple_element_t<0u, Stringable::convention_types>;
  std::cout << pro::proxy_invoke<C, std::string() const>(p) << "\n";  // Invokes with proxy_invoke, also prints: "123"
}
