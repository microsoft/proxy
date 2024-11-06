// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from PRO_DEF_FREE_AS_MEM_DISPATCH.md.

#include <iostream>
#include <string>

#include "proxy.h"

PRO_DEF_FREE_AS_MEM_DISPATCH(FreeToString, std::to_string, ToString);

struct Stringable : pro::facade_builder
    ::add_convention<FreeToString, std::string()>
    ::build {};

int main() {
  pro::proxy<Stringable> p = pro::make_proxy<Stringable>(123);
  std::cout << p->ToString() << "\n";  // Prints: "123"
}
