// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from add_reflection.md.

#include <array>
#include <iostream>

#include "proxy.h"

class LayoutReflector {
 public:
  template <class T>
  constexpr explicit LayoutReflector(std::in_place_type_t<T>)
      : size_(sizeof(T)), align_(alignof(T)) {}

  template <class F, bool IsDirect, class R>
  struct accessor {
    friend std::size_t SizeOf(const std::conditional_t<IsDirect, pro::proxy<F>,
        pro::proxy_indirect_accessor<F>>& self) noexcept {
      const LayoutReflector& refl = pro::proxy_reflect<IsDirect, R>(pro::access_proxy<F>(self));
      return refl.size_;
    }

    friend std::size_t AlignOf(const std::conditional_t<IsDirect, pro::proxy<F>,
        pro::proxy_indirect_accessor<F>>& self) noexcept {
      const LayoutReflector& refl = pro::proxy_reflect<IsDirect, R>(pro::access_proxy<F>(self));
      return refl.align_;
    }
  };

 private:
  std::size_t size_, align_;
};

struct LayoutAware : pro::facade_builder
    ::add_direct_reflection<LayoutReflector>
    ::add_indirect_reflection<LayoutReflector>
    ::build {};

int main() {
  int a = 123;
  pro::proxy<LayoutAware> p = &a;
  std::cout << SizeOf(p) << "\n";  // Prints sizeof(raw pointer)
  std::cout << AlignOf(p) << "\n";  // Prints alignof(raw pointer)
  std::cout << SizeOf(*p) << "\n";  // Prints sizeof(int)
  std::cout << AlignOf(*p) << "\n";  // Prints alignof(int)

  p = pro::make_proxy<LayoutAware>(123);  // SBO enabled
  std::cout << SizeOf(p) << "\n";  // Prints sizeof(int)
  std::cout << AlignOf(p) << "\n";  // Prints alignof(int)
  std::cout << SizeOf(*p) << "\n";  // Prints sizeof(int)
  std::cout << AlignOf(*p) << "\n";  // Prints alignof(int)

  p = pro::make_proxy<LayoutAware, std::array<char, 100>>();  // SBO disabled
  std::cout << SizeOf(p) << "\n";  // Prints sizeof(raw pointer)
  std::cout << AlignOf(p) << "\n";  // Prints alignof(raw pointer)
  std::cout << SizeOf(*p) << "\n";  // Prints "100"
  std::cout << AlignOf(*p) << "\n";  // Prints "1"
}
