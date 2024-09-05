// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from emplace.md.

#include <iostream>
#include <memory>
#include <memory_resource>

#include "proxy.h"

struct AnyCopyable : pro::facade_builder
    ::support_copy<pro::constraint_level::nontrivial>
    ::build {};

struct Foo {
  ~Foo() { puts("Destroy Foo"); }

  int payload[10000];
};

int main() {
  static std::pmr::unsynchronized_pool_resource my_memory_pool;

  std::pmr::polymorphic_allocator<> alloc{&my_memory_pool};
  auto deleter = [alloc](auto* ptr) mutable { alloc.delete_object(ptr); };

  pro::proxy<AnyCopyable> p0;
  p0.emplace<std::shared_ptr<Foo>>(alloc.new_object<Foo>(), deleter);
  pro::proxy<AnyCopyable> p1 = p0;  // `Foo` is not copied. Only the reference count is increased.
}  // The destructor of `Foo` is called once when both `p0` and `p1` are destroyed
