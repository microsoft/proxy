// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <memory>
#include <vector>

#include "proxy.h"

PRO_DEF_MEM_DISPATCH(MemFun, Fun);

struct TestFacade : pro::facade_builder
    ::add_convention<MemFun, int() const>
    ::build{};

struct TestBase {
  virtual int Fun() const = 0;
  virtual ~TestBase() = default;
};

extern const std::vector<pro::proxy<TestFacade>> SmallObjectInvocationProxyTestData;
extern const std::vector<std::unique_ptr<TestBase>> SmallObjectInvocationVirtualFunctionTestData;
extern const std::vector<pro::proxy<TestFacade>> LargeObjectInvocationProxyTestData;
extern const std::vector<std::unique_ptr<TestBase>> LargeObjectInvocationVirtualFunctionTestData;
