// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from PRO_DEF_WEAK_DISPATCH.md.

#include <iostream>
#include <string>
#include <vector>

#include "proxy.h"

struct NotImplemented {
  explicit NotImplemented(auto&&...) { throw std::runtime_error{ "Not implemented!" }; }

  template <class T>
  operator T() const noexcept { std::terminate(); }  // Or std::unreachable() in C++23
};

PRO_DEF_MEM_DISPATCH(MemAt, at);
PRO_DEF_WEAK_DISPATCH(WeakMemAt, MemAt, NotImplemented);

struct WeakDictionary : pro::facade_builder
    ::add_convention<WeakMemAt, std::string(int index) const>
    ::build {};

int main() {
  std::vector<const char*> v{"hello", "world"};
  pro::proxy<WeakDictionary> p1 = &v;
  std::cout << p1->at(1) << "\n";  // Prints: "world"
  pro::proxy<WeakDictionary> p2 = pro::make_proxy<WeakDictionary>(123);
  try {
    p2->at(1);
  } catch (const std::runtime_error& e) {
    std::cout << e.what() << "\n";  // Prints: "Not implemented!"
  }
}
