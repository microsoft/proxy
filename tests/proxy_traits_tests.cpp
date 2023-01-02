// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <type_traits>
#include "proxy.h"

namespace {

template <bool kNothrowRelocatable, bool kCopyable, bool kTrivial, std::size_t kSize, std::size_t kAlignment>
struct MockPtr {
  MockPtr() = default;
  MockPtr(int) noexcept {}
  MockPtr(const MockPtr&) requires(kCopyable && !kTrivial) {}
  MockPtr(const MockPtr&) noexcept requires(kTrivial) = default;
  MockPtr(MockPtr&&) noexcept requires(kNothrowRelocatable && !kTrivial) {}
  MockPtr(MockPtr&&) noexcept requires(kTrivial) = default;
  const MockPtr& operator*() const noexcept { return *this; }

  alignas(kAlignment) char dummy_[kSize];
};
using MockMovablePtr = MockPtr<true, false, false, sizeof(void*) * 2, alignof(void*)>;
using MockCopyablePtr = MockPtr<true, true, false, sizeof(void*) * 2, alignof(void*)>;
using MockCopyableSmallPtr = MockPtr<true, true, false, sizeof(void*), alignof(void*)>;
using MockTrivialPtr = MockPtr<true, true, true, sizeof(void*) * 2, alignof(void*)>;

using DefaultFacade = pro::facade<>;
static_assert(DefaultFacade::minimum_copyability == pro::constraint_level::none);
static_assert(DefaultFacade::minimum_relocatability == pro::constraint_level::nothrow);
static_assert(DefaultFacade::minimum_destructibility == pro::constraint_level::nothrow);
static_assert(DefaultFacade::maximum_size >= 2 * sizeof(void*));
static_assert(DefaultFacade::maximum_alignment >= sizeof(void*));
static_assert(std::is_same_v<DefaultFacade::dispatch_types, std::tuple<>>);
static_assert(std::is_same_v<DefaultFacade::reflection_type, void>);
static_assert(std::is_nothrow_default_constructible_v<pro::proxy<DefaultFacade>>);
static_assert(!std::is_trivially_default_constructible_v<pro::proxy<DefaultFacade>>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<DefaultFacade>, std::nullptr_t>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<DefaultFacade>, std::nullptr_t>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<DefaultFacade>, std::in_place_type_t<MockMovablePtr>, int>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<DefaultFacade>, std::in_place_type_t<MockCopyablePtr>, int>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<DefaultFacade>, std::in_place_type_t<MockCopyableSmallPtr>, int>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<DefaultFacade>, std::in_place_type_t<MockTrivialPtr>, int>);

struct RelocatableFacade : pro::facade<> {};
static_assert(!std::is_copy_constructible_v<pro::proxy<RelocatableFacade>>);
static_assert(!std::is_copy_assignable_v<pro::proxy<RelocatableFacade>>);
static_assert(std::is_nothrow_move_constructible_v<pro::proxy<RelocatableFacade>>);
static_assert(!std::is_trivially_move_constructible_v<pro::proxy<RelocatableFacade>>);
static_assert(std::is_nothrow_move_assignable_v<pro::proxy<RelocatableFacade>>);
static_assert(!std::is_trivially_move_assignable_v<pro::proxy<RelocatableFacade>>);
static_assert(std::is_nothrow_destructible_v<pro::proxy<RelocatableFacade>>);
static_assert(!std::is_trivially_destructible_v<pro::proxy<RelocatableFacade>>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<RelocatableFacade>, MockMovablePtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<RelocatableFacade>, MockMovablePtr>);
static_assert(pro::proxiable<MockMovablePtr, RelocatableFacade>);
static_assert(pro::proxiable<MockCopyablePtr, RelocatableFacade>);
static_assert(pro::proxiable<MockCopyableSmallPtr, RelocatableFacade>);
static_assert(pro::proxiable<MockTrivialPtr, RelocatableFacade>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<RelocatableFacade>, MockMovablePtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<RelocatableFacade>, MockMovablePtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<RelocatableFacade>, MockCopyablePtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<RelocatableFacade>, MockCopyablePtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<RelocatableFacade>, MockCopyableSmallPtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<RelocatableFacade>, MockCopyableSmallPtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<RelocatableFacade>, MockTrivialPtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<RelocatableFacade>, MockTrivialPtr>);

struct CopyableFacade : pro::facade<> {
  static constexpr auto minimum_copyability = pro::constraint_level::nontrivial;
};
static_assert(std::is_copy_constructible_v<pro::proxy<CopyableFacade>>);
static_assert(!std::is_nothrow_copy_constructible_v<pro::proxy<CopyableFacade>>);
static_assert(std::is_copy_assignable_v<pro::proxy<CopyableFacade>>);
static_assert(!std::is_nothrow_copy_assignable_v<pro::proxy<CopyableFacade>>);
static_assert(std::is_nothrow_move_constructible_v<pro::proxy<CopyableFacade>>);
static_assert(!std::is_trivially_move_constructible_v<pro::proxy<CopyableFacade>>);
static_assert(std::is_nothrow_move_assignable_v<pro::proxy<CopyableFacade>>);
static_assert(!std::is_trivially_move_assignable_v<pro::proxy<CopyableFacade>>);
static_assert(std::is_nothrow_destructible_v<pro::proxy<CopyableFacade>>);
static_assert(!std::is_trivially_destructible_v<pro::proxy<CopyableFacade>>);
static_assert(!pro::proxiable<MockMovablePtr, CopyableFacade>);
static_assert(pro::proxiable<MockCopyablePtr, CopyableFacade>);
static_assert(pro::proxiable<MockCopyableSmallPtr, CopyableFacade>);
static_assert(pro::proxiable<MockTrivialPtr, CopyableFacade>);
static_assert(!std::is_constructible_v<pro::proxy<CopyableFacade>, MockMovablePtr>);
static_assert(!std::is_assignable_v<pro::proxy<CopyableFacade>, MockMovablePtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<CopyableFacade>, MockCopyablePtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<CopyableFacade>, MockCopyablePtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<CopyableFacade>, MockCopyableSmallPtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<CopyableFacade>, MockCopyableSmallPtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<CopyableFacade>, MockTrivialPtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<CopyableFacade>, MockTrivialPtr>);

struct CopyableSmallFacade : pro::facade<> {
  static constexpr std::size_t maximum_size = sizeof(void*);
  static constexpr auto minimum_copyability = pro::constraint_level::nontrivial;
};
static_assert(!pro::proxiable<MockMovablePtr, CopyableSmallFacade>);
static_assert(!pro::proxiable<MockCopyablePtr, CopyableSmallFacade>);
static_assert(pro::proxiable<MockCopyableSmallPtr, CopyableSmallFacade>);
static_assert(!pro::proxiable<MockTrivialPtr, CopyableSmallFacade>);
static_assert(!std::is_constructible_v<pro::proxy<CopyableSmallFacade>, MockMovablePtr>);
static_assert(!std::is_assignable_v<pro::proxy<CopyableSmallFacade>, MockMovablePtr>);
static_assert(!std::is_constructible_v<pro::proxy<CopyableSmallFacade>, MockCopyablePtr>);
static_assert(!std::is_assignable_v<pro::proxy<CopyableSmallFacade>, MockCopyablePtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<CopyableSmallFacade>, MockCopyableSmallPtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<CopyableSmallFacade>, MockCopyableSmallPtr>);
static_assert(!std::is_constructible_v<pro::proxy<CopyableSmallFacade>, MockTrivialPtr>);
static_assert(!std::is_assignable_v<pro::proxy<CopyableSmallFacade>, MockTrivialPtr>);

struct TrivialFacade : pro::facade<> {
  static constexpr auto minimum_copyability = pro::constraint_level::trivial;
  static constexpr auto minimum_relocatability = pro::constraint_level::trivial;
  static constexpr auto minimum_destructibility = pro::constraint_level::trivial;
};
static_assert(std::is_trivially_copy_constructible_v<pro::proxy<TrivialFacade>>);
static_assert(std::is_trivially_copy_assignable_v<pro::proxy<TrivialFacade>>);
static_assert(std::is_nothrow_move_constructible_v<pro::proxy<TrivialFacade>>);
static_assert(!std::is_trivially_move_constructible_v<pro::proxy<TrivialFacade>>);
static_assert(std::is_nothrow_move_assignable_v<pro::proxy<TrivialFacade>>);
static_assert(!std::is_trivially_move_assignable_v<pro::proxy<TrivialFacade>>);
static_assert(std::is_trivially_destructible_v<pro::proxy<TrivialFacade>>);
static_assert(!pro::proxiable<MockMovablePtr, TrivialFacade>);
static_assert(!pro::proxiable<MockCopyablePtr, TrivialFacade>);
static_assert(!pro::proxiable<MockCopyableSmallPtr, TrivialFacade>);
static_assert(pro::proxiable<MockTrivialPtr, TrivialFacade>);
static_assert(!std::is_constructible_v<pro::proxy<TrivialFacade>, MockMovablePtr>);
static_assert(!std::is_assignable_v<pro::proxy<TrivialFacade>, MockMovablePtr>);
static_assert(!std::is_constructible_v<pro::proxy<TrivialFacade>, MockCopyablePtr>);
static_assert(!std::is_assignable_v<pro::proxy<TrivialFacade>, MockCopyablePtr>);
static_assert(!std::is_constructible_v<pro::proxy<TrivialFacade>, MockCopyableSmallPtr>);
static_assert(!std::is_assignable_v<pro::proxy<TrivialFacade>, MockCopyableSmallPtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<TrivialFacade>, MockTrivialPtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<TrivialFacade>, MockTrivialPtr>);

}  // namespace
