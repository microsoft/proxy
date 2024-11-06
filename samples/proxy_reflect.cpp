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

  PRO_DEF_REFL_AS_MEM_ACCESSOR(ReflectTraits);

  const bool Copyable;
};

struct TestFacade : pro::facade_builder
    ::add_direct_reflection<TraitsRefl>
    ::build {};

int main() {
  pro::proxy<TestFacade> p1 = std::make_unique<int>();
  std::cout << std::boolalpha << p1.ReflectTraits().Copyable << "\n";  // Reflects with accessor, prints: "false"
  using R = std::tuple_element_t<0u, TestFacade::reflection_types>;
  std::cout << pro::proxy_reflect<R>(p1).Copyable << "\n";  // Reflects with proxy_reflect, also prints: "false"

  pro::proxy<TestFacade> p2 = std::make_shared<int>();
  std::cout << p2.ReflectTraits().Copyable << "\n";  // Reflects with accessor, prints: "true"
  std::cout << pro::proxy_reflect<R>(p2).Copyable << "\n";  // Reflects with proxy_reflect, also prints: "true"
}
