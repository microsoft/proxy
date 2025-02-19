// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from proxy_view.md.

#include <map>

#include "proxy.h"

template <class K, class V>
struct FMap : pro::facade_builder
    ::add_convention<pro::operator_dispatch<"[]">, V&(const K& key)>
    ::build {};

int main() {
  std::map<int, int> v;
  pro::proxy_view<FMap<int, int>> p = &v;
  (*p)[1] = 123;
  printf("%d\n", v.at(1));  // Prints "123"
}
