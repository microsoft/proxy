// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from restrict_layout.md.

#include <array>
#include <memory>

#include "proxy.h"

struct DefaultFacade : pro::facade_builder::build {};

struct SmallFacade : pro::facade_builder
    ::restrict_layout<sizeof(void*)>
    ::build {};

int main() {
  static_assert(sizeof(pro::proxy<DefaultFacade>) > sizeof(pro::proxy<SmallFacade>));
  static_assert(pro::proxiable<std::unique_ptr<int>, DefaultFacade>);
  static_assert(pro::proxiable<std::unique_ptr<int>, SmallFacade>);
  static_assert(pro::proxiable<std::shared_ptr<int>, DefaultFacade>);
  static_assert(!pro::proxiable<std::shared_ptr<int>, SmallFacade>);
  static_assert(pro::inplace_proxiable_target<std::array<int*, 2>, DefaultFacade>);
  static_assert(!pro::inplace_proxiable_target<std::array<int*, 2>, SmallFacade>);
  static_assert(!pro::inplace_proxiable_target<std::array<int*, 3>, DefaultFacade>);
  static_assert(!pro::inplace_proxiable_target<std::array<int*, 3>, SmallFacade>);
  pro::proxy<DefaultFacade> p1 = std::make_shared<int>(123);
  // pro::proxy<SmallFacade> p2 = std::make_shared<int>(123);  // Won't compile
}
