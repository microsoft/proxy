// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from basic_facade_builder.md.

#include <iostream>

#include "proxy.h"

template <class... Overloads>
struct MovableCallable : pro::facade_builder
    ::add_convention<pro::operator_dispatch<"()">, Overloads...>
    ::build {};

template <class... Overloads>
struct CopyableCallable : pro::facade_builder
    ::support_copy<pro::constraint_level::nontrivial>
    ::add_facade<MovableCallable<Overloads...>>
    ::build {};

// MyFunction has similar functionality as std::function but supports multiple overloads
// MyMoveOnlyFunction has similar functionality as std::move_only_function but supports multiple overloads
template <class... Overloads>
using MyFunction = pro::proxy<CopyableCallable<Overloads...>>;
template <class... Overloads>
using MyMoveOnlyFunction = pro::proxy<MovableCallable<Overloads...>>;

int main() {
  auto f = [](auto&&... v) {
    std::cout << "f() called. Args: ";
    ((std::cout << v << ":" << typeid(decltype(v)).name() << ", "), ...);
    std::cout << "\n";
  };
  MyFunction<void(int)> p0{&f};
  (*p0)(123);  // Prints "f() called. Args: 123:i," (assuming GCC)
  MyMoveOnlyFunction<void(), void(int), void(double)> p1{&f};
  (*p1)();  // Prints "f() called. Args:"
  (*p1)(456);  // Prints "f() called. Args: 456:i,"
  (*p1)(1.2);  // Prints "f() called. Args: 1.2:d,"
}
