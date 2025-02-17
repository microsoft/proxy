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

template <class T, class F>
pro::proxy<F> operator*(const T& value, const pro::proxy_indirect_accessor<F>& rhs)
    requires(!std::is_same_v<T, pro::proxy_indirect_accessor<F>>)
    { return pro::make_proxy<F, T>(value * proxy_cast<const T&>(rhs)); }

struct Addable : pro::facade_builder
    ::support_rtti
    ::add_convention<pro::operator_dispatch<"+">, pro::facade_aware_overload_t<BinaryOverload>>
    ::build {};

struct Multipliable : pro::facade_builder
    ::support_rtti
    ::add_convention<pro::operator_dispatch<"*">, pro::facade_aware_overload_t<BinaryOverload>>
    ::build {};

struct BasicNumber : pro::facade_builder
    ::support_format
    ::add_facade<Addable>
    ::add_facade<Multipliable>
    ::build {};

int main() {
  pro::proxy<BasicNumber> p1 = pro::make_proxy<BasicNumber>(1);
  pro::proxy<BasicNumber> p2 = pro::make_proxy<BasicNumber>(2);
  pro::proxy<BasicNumber> p3 = *p1 + *p2;  // p3 is int(3)
  p3 = *p3 * *p2;  // p3 becomes int(6)
  std::cout << std::format("{}\n", *p3);  // Prints "6"
}
