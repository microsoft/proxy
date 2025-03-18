// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from allocate_proxy_shared.md.

#include <iostream>
#include <memory_resource>

#include "proxy.h"

struct RttiAware : pro::facade_builder
    ::support_copy<pro::constraint_level::nothrow>
    ::support_rtti
    ::build {};

int main() {
  std::pmr::unsynchronized_pool_resource pool;
  std::pmr::polymorphic_allocator<> alloc{&pool};
  pro::proxy<RttiAware> p1 = pro::allocate_proxy_shared<RttiAware>(alloc, 1);
  pro::proxy<RttiAware> p2 = p1;
  proxy_cast<int&>(*p1) += 2;
  std::cout << proxy_cast<int>(*p2) << "\n";  // Prints "3"
}
