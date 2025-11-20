// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <any>
#include <memory>
#include <vector>

#include <proxy/proxy.h>

PRO_DEF_MEM_DISPATCH(MemFun, Fun);

struct InvocationTestFacade : pro::facade_builder                   //
                              ::add_convention<MemFun, int() const> //
                              ::add_skill<pro::skills::as_view>     //
                              ::add_skill<pro::skills::slim>        //
                              ::build {};

struct NothrowRelocatableInvocationTestFacade : InvocationTestFacade {
  static constexpr auto relocatability = pro::constraint_level::nothrow;
};

struct InvocationTestBase {
  virtual int Fun() const = 0;
  virtual ~InvocationTestBase() = default;
};

std::vector<pro::proxy<InvocationTestFacade>>
    GenerateSmallObjectProxyTestData();
std::vector<pro::proxy<NothrowRelocatableInvocationTestFacade>>
    GenerateSmallObjectProxyTestData_NothrowRelocatable();
std::vector<pro::proxy<InvocationTestFacade>>
    GenerateSmallObjectProxyTestData_Shared();
std::vector<std::unique_ptr<InvocationTestBase>>
    GenerateSmallObjectVirtualFunctionTestData();
std::vector<std::shared_ptr<InvocationTestBase>>
    GenerateSmallObjectVirtualFunctionTestData_Shared();
std::vector<std::any> GenerateSmallObjectAnyTestData();

std::vector<pro::proxy<InvocationTestFacade>>
    GenerateLargeObjectProxyTestData();
std::vector<pro::proxy<NothrowRelocatableInvocationTestFacade>>
    GenerateLargeObjectProxyTestData_NothrowRelocatable();
std::vector<pro::proxy<InvocationTestFacade>>
    GenerateLargeObjectProxyTestData_Shared();
std::vector<std::unique_ptr<InvocationTestBase>>
    GenerateLargeObjectVirtualFunctionTestData();
std::vector<std::shared_ptr<InvocationTestBase>>
    GenerateLargeObjectVirtualFunctionTestData_Shared();
std::vector<std::any> GenerateLargeObjectAnyTestData();
