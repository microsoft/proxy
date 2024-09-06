// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from indirection.md.

#include <iostream>
#include <string>
#include <vector>

#include "proxy.h"

PRO_DEF_MEM_DISPATCH(MemSize, size);

struct BasicContainer : pro::facade_builder
    ::add_convention<MemSize, std::size_t() const& noexcept>
    ::build {};

PRO_DEF_FREE_DISPATCH(FreeToString, std::to_string, ToString);

struct Stringable : pro::facade_builder
    ::add_convention<FreeToString, std::string()>
    ::build {};

int main() {
  std::vector<int> v(10);
  pro::proxy<BasicContainer> p0 = &v;
  std::cout << p0->size() << "\n";  // Prints "10"
  std::cout << (*p0).size() << "\n";  // Prints "10"

  pro::proxy<Stringable> p1 = pro::make_proxy<Stringable>(123);
  std::cout << ToString(*p1) << "\n";  // Prints "123"
}
