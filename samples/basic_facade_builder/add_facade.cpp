// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from add_facade.md.

#include <iostream>
#include <unordered_map>

#include "proxy.h"

PRO_DEF_MEM_DISPATCH(MemSize, size);
PRO_DEF_MEM_DISPATCH(MemAt, at);
PRO_DEF_MEM_DISPATCH(MemEmplace, emplace);

struct Copyable : pro::facade_builder
    ::support_copy<pro::constraint_level::nontrivial>
    ::build {};

struct BasicContainer : pro::facade_builder
    ::add_convention<MemSize, std::size_t() const noexcept>
    ::build {};

struct StringDictionary : pro::facade_builder
    ::add_facade<BasicContainer>
    ::add_facade<Copyable>
    ::add_convention<MemAt, std::string(std::size_t key) const>
    ::build {};

struct MutableStringDictionary : pro::facade_builder
    ::add_facade<StringDictionary, true>
    ::add_convention<MemEmplace, void(std::size_t key, std::string value)>
    ::build {};

int main() {
  pro::proxy<MutableStringDictionary> p1 =
      pro::make_proxy<MutableStringDictionary, std::unordered_map<std::size_t, std::string>>();
  std::cout << p1->size() << "\n";  // Prints "0"
  try {
    std::cout << p1->at(123) << "\n";  // No output because the expression throws
  } catch (const std::out_of_range& e) {
    std::cerr << e.what() << "\n";  // Prints error message
  }
  p1->emplace(123, "lalala");
  auto p2 = p1;  // Performs a deep copy
  p2->emplace(456, "trivial");
  pro::proxy<StringDictionary> p3 = std::move(p2);  // Performs an upward conversion from an rvalue reference
  std::cout << p1->size() << "\n";  // Prints "1"
  std::cout << p1->at(123) << "\n";  // Prints "lalala"
  std::cout << std::boolalpha << p2.has_value() << "\n";  // Prints "false" because it is moved
  std::cout << p3->size() << "\n";  // Prints "2"
  std::cout << p3->at(123) << "\n";  // Prints "lalala"
  std::cout << p3->at(456) << "\n";  // Prints "trivial"
}
