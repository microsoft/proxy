// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <memory>
#include <vector>

#include "proxy.h"

PRO_DEF_MEM_DISPATCH(MemFun, Fun);

struct InvocationTestFacade : pro::facade_builder
    ::add_convention<MemFun, int() const>
    ::build{};

struct InvocationTestBase {
  virtual int Fun() const = 0;
  virtual ~InvocationTestBase() = default;
};

std::vector<pro::proxy<InvocationTestFacade>> GenerateSmallObjectInvocationProxyTestData();
std::vector<std::unique_ptr<InvocationTestBase>> GenerateSmallObjectInvocationVirtualFunctionTestData();
std::vector<pro::proxy<InvocationTestFacade>> GenerateLargeObjectInvocationProxyTestData();
std::vector<std::unique_ptr<InvocationTestBase>> GenerateLargeObjectInvocationVirtualFunctionTestData();
