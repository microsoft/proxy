// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from PRO_DEF_MEM_DISPATCH.md.

#include <iostream>
#include <string>
#include <vector>

#include "proxy.h"

PRO_DEF_MEM_DISPATCH(MemAt, at);

struct Dictionary : pro::facade_builder
    ::add_convention<MemAt, std::string(int index) const>
    ::build {};

int main() {
  std::vector<const char*> v{"hello", "world"};
  pro::proxy<Dictionary> p = &v;
  std::cout << p->at(1) << "\n";  // Prints: "world"
}
