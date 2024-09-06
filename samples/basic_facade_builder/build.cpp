// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from build.md.

#include <type_traits>

#include "proxy.h"

struct DefaultBase : pro::facade_builder
    ::build {};

struct CopyableBase : pro::facade_builder
    ::support_copy<pro::constraint_level::nontrivial>
    ::build {};

struct TrivialBase : pro::facade_builder
    ::support_copy<pro::constraint_level::trivial>
    ::support_relocation<pro::constraint_level::trivial>
    ::support_destruction<pro::constraint_level::trivial>
    ::restrict_layout<sizeof(void*)>
    ::build {};

int main() {
  static_assert(!std::is_copy_constructible_v<pro::proxy<DefaultBase>>);
  static_assert(std::is_nothrow_move_constructible_v<pro::proxy<DefaultBase>>);
  static_assert(std::is_nothrow_destructible_v<pro::proxy<DefaultBase>>);

  static_assert(std::is_copy_constructible_v<pro::proxy<CopyableBase>>);
  static_assert(std::is_nothrow_move_constructible_v<pro::proxy<CopyableBase>>);
  static_assert(std::is_nothrow_destructible_v<pro::proxy<CopyableBase>>);

  static_assert(std::is_trivially_copy_constructible_v<pro::proxy<TrivialBase>>);
  static_assert(std::is_trivially_move_constructible_v<pro::proxy<TrivialBase>>);
  static_assert(std::is_trivially_destructible_v<pro::proxy<TrivialBase>>);
}
