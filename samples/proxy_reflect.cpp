// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from proxy_reflect.md.

#include <iostream>
#include <memory>
#include <type_traits>

#include "proxy.h"

struct TraitsRefl {
  template <class P>
  constexpr explicit TraitsRefl(std::in_place_type_t<P>)
      : Copyable(std::is_copy_constructible_v<P>) {}

  const bool Copyable;
};

struct TestFacade : pro::facade_builder
    ::add_reflection<TraitsRefl>
    ::build {};

int main() {
  pro::proxy<TestFacade> p1 = std::make_unique<int>();
  std::cout << std::boolalpha << pro::proxy_reflect<TraitsRefl>(p1).Copyable << "\n";  // Prints: "false"

  pro::proxy<TestFacade> p2 = std::make_shared<int>();
  std::cout << pro::proxy_reflect<TraitsRefl>(p2).Copyable << "\n";  // Prints: "true"
}
