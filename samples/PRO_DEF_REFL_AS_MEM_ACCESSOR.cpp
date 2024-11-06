// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from PRO_DEF_REFL_AS_MEM_ACCESSOR.md.

#include <iostream>
#include <typeinfo>

#include "proxy.h"

class RttiReflector {
 public:
  template <class T>
  constexpr explicit RttiReflector(std::in_place_type_t<T>) : type_(typeid(T)) {}

  PRO_DEF_REFL_AS_MEM_ACCESSOR(ReflectRtti);
  const char* GetName() const noexcept { return type_.name(); }

 private:
  const std::type_info& type_;
};

struct RttiAware : pro::facade_builder
    ::add_reflection<RttiReflector>
    ::build {};

int main() {
  pro::proxy<RttiAware> p = pro::make_proxy<RttiAware>(123);
  std::cout << p->ReflectRtti().GetName() << "\n";  // Prints: "i" (assuming GCC)
}
