// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from weak_dispatch.md.

#include <iostream>
#include <string>
#include <vector>

#include "proxy.h"

PRO_DEF_MEM_DISPATCH(MemAt, at);

struct WeakDictionary : pro::facade_builder
    ::add_convention<pro::weak_dispatch<MemAt>, std::string(int index) const>
    ::build {};

int main() {
  std::vector<const char*> v{"hello", "world"};
  pro::proxy<WeakDictionary> p1 = &v;
  std::cout << p1->at(1) << "\n";  // Prints: "world"
  pro::proxy<WeakDictionary> p2 = pro::make_proxy<WeakDictionary>(123);
  try {
    p2->at(1);
  } catch (const pro::not_implemented& e) {
    std::cout << e.what() << "\n";  // Prints an explanatory string
  }
}
