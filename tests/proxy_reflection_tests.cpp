// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "proxy.h"

namespace {

class MyReflectionInfo {
 public:
  template <class P>
  constexpr explicit MyReflectionInfo(std::in_place_type_t<P>)
      : type_(typeid(P)) {}

  const char* GetName() const noexcept { return type_.name(); }

 private:
  const std::type_info& type_;
};
struct TestFacade : pro::facade<> {
  using reflection_type = MyReflectionInfo;
};

}  // namespace