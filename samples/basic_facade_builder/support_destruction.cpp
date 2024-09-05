// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from support_destruction.md.

#include <type_traits>

#include "proxy.h"

struct Movable : pro::facade_builder::build {};

struct NonriviallyDestructible : pro::facade_builder
    ::support_relocation<pro::constraint_level::nontrivial>
    ::support_destruction<pro::constraint_level::nontrivial>
    ::build {};

int main() {
  static_assert(std::is_nothrow_destructible_v<pro::proxy<Movable>>);
  static_assert(!std::is_nothrow_destructible_v<pro::proxy<NonriviallyDestructible>>);
}
