// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <memory>
#include <memory_resource>
#include <vector>

#include "proxy.h"

namespace details {

extern std::pmr::unsynchronized_pool_resource InvocationBenchmarkMemoryPool;

struct InvocationBenchmarkPolledDeleter {
  void operator()(auto* ptr) const noexcept {
    std::pmr::polymorphic_allocator<> alloc{&InvocationBenchmarkMemoryPool};
    alloc.delete_object(ptr);
  }
};

}  // namespace details

PRO_DEF_MEM_DISPATCH(MemFun, Fun);

struct InvocationTestFacade : pro::facade_builder
    ::add_convention<MemFun, int() const>
    ::build{};

struct InvocationTestBase {
  virtual int Fun() const = 0;
  virtual ~InvocationTestBase() = default;
};

extern const std::vector<pro::proxy<InvocationTestFacade>> SmallObjectInvocationProxyTestData;
extern const std::vector<std::unique_ptr<InvocationTestBase>> SmallObjectInvocationVirtualFunctionTestData;
extern const std::vector<std::unique_ptr<InvocationTestBase, details::InvocationBenchmarkPolledDeleter>> PooledSmallObjectInvocationVirtualFunctionTestData;
extern const std::vector<pro::proxy<InvocationTestFacade>> LargeObjectInvocationProxyTestData;
extern const std::vector<std::unique_ptr<InvocationTestBase>> LargeObjectInvocationVirtualFunctionTestData;
extern const std::vector<pro::proxy<InvocationTestFacade>> PooledLargeObjectInvocationProxyTestData;
extern const std::vector<std::unique_ptr<InvocationTestBase, details::InvocationBenchmarkPolledDeleter>> PooledLargeObjectInvocationVirtualFunctionTestData;
