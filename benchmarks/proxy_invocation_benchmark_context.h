// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <memory>
#include <vector>

#include <proxy/proxy.h>

PRO_DEF_MEM_DISPATCH(MemFun, Fun);

struct InvocationTestFacade : pro::facade_builder                   //
                              ::add_convention<MemFun, int() const> //
                              ::support<pro::skills::as_view>       //
                              ::build {};

struct InvocationTestBase {
  virtual int Fun() const = 0;
  virtual ~InvocationTestBase() = default;
};

std::vector<pro::proxy<InvocationTestFacade>>
    GenerateSmallObjectInvocationProxyTestData();
std::vector<pro::proxy<InvocationTestFacade>>
    GenerateSmallObjectInvocationProxyTestData_Shared();
std::vector<std::unique_ptr<InvocationTestBase>>
    GenerateSmallObjectInvocationVirtualFunctionTestData();
std::vector<std::shared_ptr<InvocationTestBase>>
    GenerateSmallObjectInvocationVirtualFunctionTestData_Shared();
std::vector<pro::proxy<InvocationTestFacade>>
    GenerateLargeObjectInvocationProxyTestData();
std::vector<pro::proxy<InvocationTestFacade>>
    GenerateLargeObjectInvocationProxyTestData_Shared();
std::vector<std::unique_ptr<InvocationTestBase>>
    GenerateLargeObjectInvocationVirtualFunctionTestData();
std::vector<std::shared_ptr<InvocationTestBase>>
    GenerateLargeObjectInvocationVirtualFunctionTestData_Shared();
