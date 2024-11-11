// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from proxy_reflect.md.

#include <iostream>
#include <memory>
#include <type_traits>

#include "proxy.h"

class CopyabilityReflector {
 public:
  template <class T>
  constexpr explicit CopyabilityReflector(std::in_place_type_t<T>)
      : copyable_(std::is_copy_constructible_v<T>) {}

  template <class F, class R>
  struct accessor {
    bool IsCopyable() const noexcept {
      const CopyabilityReflector& self = pro::proxy_reflect<R>(pro::access_proxy<F>(*this));
      return self.copyable_;
    }
  };

 private:
  bool copyable_;
};

struct CopyabilityAware : pro::facade_builder
    ::add_direct_reflection<CopyabilityReflector>
    ::build {};

int main() {
  pro::proxy<CopyabilityAware> p1 = std::make_unique<int>();
  std::cout << std::boolalpha << p1.IsCopyable() << "\n";  // Prints: "false"

  pro::proxy<CopyabilityAware> p2 = std::make_shared<int>();
  std::cout << p2.IsCopyable() << "\n";  // Prints: "true"
}
