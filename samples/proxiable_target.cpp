// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from proxiable_target.md.

#include "proxy.h"

struct Runnable : pro::facade_builder
    ::add_convention<pro::operator_dispatch<"()">, void()>
    ::build {};

int main() {
  auto fun = [] {};
  static_assert(pro::proxiable_target<decltype(fun), Runnable>);
  static_assert(!pro::proxiable_target<int, Runnable>);
}
