// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from proxy.md.

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "proxy.h"

PRO_DEF_MEM_DISPATCH(MemAt, at);

struct Dictionary : pro::facade_builder
    ::add_convention<MemAt, std::string(int)>
    ::build {};

// This is a function, rather than a function template
void PrintDictionary(pro::proxy<Dictionary> dictionary) {
  std::cout << dictionary->at(1) << "\n";
}

int main() {
  static std::map<int, std::string> container1{{1, "hello"}};
  auto container2 = std::make_shared<std::vector<const char*>>();
  container2->push_back("hello");
  container2->push_back("world");
  PrintDictionary(&container1);  // Prints: "hello"
  PrintDictionary(container2);  // Prints: "world"
}
