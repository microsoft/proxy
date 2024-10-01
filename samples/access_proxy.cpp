// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from access_proxy.md.

#include <iostream>
#include <string>

#include "proxy.h"

PRO_DEF_FREE_DISPATCH(FreeToString, std::to_string, ToString);

struct Stringable : pro::facade_builder
    ::add_convention<FreeToString, std::string()>
    ::build {};

int main() {
  pro::proxy<Stringable> p = pro::make_proxy<Stringable>(123);

  // Invokes with accessibility API
  std::cout << ToString(*p) << "\n";  // Prints: "123"

  // How it works behind the scenes
  using Convention = std::tuple_element_t<0u, Stringable::convention_types>;
  using Accessor = Convention::accessor<Stringable>;
  static_assert(std::is_base_of_v<Accessor, std::remove_reference_t<decltype(*p)>>);
  Accessor& a = static_cast<Accessor&>(*p);
  pro::proxy<Stringable>& p2 = pro::access_proxy<Stringable>(a);
  std::cout << std::boolalpha << (&p == &p2) << "\n";  // Prints: "true" because access_proxy converts
                                                       // an accessor back to the original proxy
  auto result = pro::proxy_invoke<Convention, std::string()>(p2);
  std::cout << result << "\n";  // Prints: "123"
}
