// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from destructor.md.

#include <cstdio>

#include "proxy.h"

struct AnyMovable : pro::facade_builder::build {};

struct Foo {
  ~Foo() { puts("Destroy Foo"); }
};

int main() {
  pro::proxy<AnyMovable> p = pro::make_proxy<AnyMovable, Foo>();
}  // The destructor of `Foo` is called when `p` is destroyed
