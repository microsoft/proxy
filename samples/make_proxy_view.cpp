// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from make_proxy_view.md.

#include <iostream>
#include <map>
#include <string>

#include "proxy.h"

PRO_DEF_MEM_DISPATCH(MemAt, at);

struct ResourceDictionary : pro::facade_builder
    ::add_convention<MemAt, std::string&(int index), const std::string&(int index) const>
    ::build {};

int main() {
  std::map<int, std::string> dict;
  dict[1] = "init";
  pro::proxy_view<ResourceDictionary> pv = pro::make_proxy_view<ResourceDictionary>(dict);
  static_assert(std::is_same_v<decltype(pv->at(1)), std::string&>, "Non-const overload");
  static_assert(std::is_same_v<decltype(std::as_const(pv)->at(1)), const std::string&>, "Const overload");
  std::cout << std::as_const(pv)->at(1) << "\n";  // Invokes the const overload and prints "init"
  pv->at(1) = "modified";  // Invokes the non-const overload
  std::cout << std::as_const(pv)->at(1) << "\n";  // Invokes the const overload and prints "modified"
}
