// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from proxiable.md.

#include <string>
#include <vector>

#include "proxy.h"

PRO_DEF_FREE_DISPATCH(FreeToString, std::to_string, ToString);

struct Stringable : pro::facade_builder
    ::add_convention<FreeToString, std::string()>
    ::build {};

int main() {
  static_assert(pro::proxiable<int*, Stringable>);
  static_assert(pro::proxiable<std::shared_ptr<double>, Stringable>);
  static_assert(!pro::proxiable<std::vector<int>*, Stringable>);
}
