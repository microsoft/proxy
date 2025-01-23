// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from observer_facade.md.

#include <map>

#include "proxy.h"

PRO_DEF_MEM_DISPATCH(MemAt, at);

template <class K, class V>
struct FMap : pro::facade_builder
    ::add_convention<MemAt, V&(const K& key), const V&(const K& key) const>
    ::build {};

int main() {
  std::map<int, int> v{{1, 2}};

  pro::proxy_view<FMap<int, int>> p1 = &v;
  static_assert(std::is_same_v<decltype(p1->at(1)), int&>);
  p1->at(1) = 3;
  printf("%d\n", v.at(1));  // Prints "3"

  pro::proxy_view<const FMap<int, int>> p2 = &std::as_const(v);
  static_assert(std::is_same_v<decltype(p2->at(1)), const int&>);
  // p2->at(1) = 4; won't compile
  printf("%d\n", p2->at(1));  // Prints "3"
}
