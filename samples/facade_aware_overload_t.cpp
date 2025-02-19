// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from facade_aware_overload_t.md.

#include <iostream>

#include "proxy.h"

template <class F>
using BinaryOverload = pro::proxy<F>(const pro::proxy_indirect_accessor<F>& rhs) const;

template <class T, class F>
pro::proxy<F> operator+(const T& value, const pro::proxy_indirect_accessor<F>& rhs)
    requires(!std::is_same_v<T, pro::proxy_indirect_accessor<F>>)
    { return pro::make_proxy<F, T>(value + proxy_cast<const T&>(rhs)); }

struct Addable : pro::facade_builder
    ::support_rtti
    ::support_format
    ::add_convention<pro::operator_dispatch<"+">, pro::facade_aware_overload_t<BinaryOverload>>
    ::build {};

int main() {
  pro::proxy<Addable> p1 = pro::make_proxy<Addable>(1);
  pro::proxy<Addable> p2 = pro::make_proxy<Addable>(2);
  pro::proxy<Addable> p3 = *p1 + *p2;
  std::cout << std::format("{}\n", *p3);  // Prints "3"
}
