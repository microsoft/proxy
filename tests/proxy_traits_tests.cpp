// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <array>
#include <stdexcept>
#include <type_traits>
#include "proxy.h"
#include "utils.h"

namespace {

template <bool kNothrowRelocatable, bool kCopyable, bool kTrivial, std::size_t kSize, std::size_t kAlignment>
struct MockPtr {
  MockPtr() = default;
  MockPtr(int) noexcept {}
  MockPtr(const MockPtr&) requires(kCopyable && !kTrivial) {}
  MockPtr(const MockPtr&) noexcept requires(kTrivial) = default;
  MockPtr(MockPtr&&) noexcept requires(kNothrowRelocatable && !kTrivial) {}
  MockPtr(MockPtr&&) noexcept requires(kTrivial) = default;
  const MockPtr* operator->() const noexcept { return this; }

  alignas(kAlignment) char dummy_[kSize];
};
using MockMovablePtr = MockPtr<true, false, false, sizeof(void*) * 2, alignof(void*)>;
using MockCopyablePtr = MockPtr<true, true, false, sizeof(void*) * 2, alignof(void*)>;
using MockCopyableSmallPtr = MockPtr<true, true, false, sizeof(void*), alignof(void*)>;
using MockTrivialPtr = MockPtr<true, true, true, sizeof(void*), alignof(void*)>;
using MockFunctionPtr = void(*)();

PRO_DEF_FACADE(DefaultFacade);
static_assert(DefaultFacade::constraints.copyability == pro::constraint_level::none);
static_assert(DefaultFacade::constraints.relocatability == pro::constraint_level::nothrow);
static_assert(DefaultFacade::constraints.destructibility == pro::constraint_level::nothrow);
static_assert(DefaultFacade::constraints.max_size >= 2 * sizeof(void*));
static_assert(DefaultFacade::constraints.max_align >= sizeof(void*));
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
static_assert(std::is_nothrow_constructible_v<pro::proxy<DefaultFacade>, std::in_place_type_t<MockFunctionPtr>, MockFunctionPtr>);
static_assert(sizeof(pro::proxy<DefaultFacade>) == 4 * sizeof(void*));  // VTABLE should be embeded

PRO_DEF_FACADE(RelocatableFacade);
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
static_assert(pro::proxiable<MockFunctionPtr, RelocatableFacade>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<RelocatableFacade>, MockMovablePtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<RelocatableFacade>, MockMovablePtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<RelocatableFacade>, MockCopyablePtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<RelocatableFacade>, MockCopyablePtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<RelocatableFacade>, MockCopyableSmallPtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<RelocatableFacade>, MockCopyableSmallPtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<RelocatableFacade>, MockTrivialPtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<RelocatableFacade>, MockTrivialPtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<RelocatableFacade>, MockFunctionPtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<RelocatableFacade>, MockFunctionPtr>);
static_assert(sizeof(pro::proxy<RelocatableFacade>) == 4 * sizeof(void*));  // VTABLE should be embeded

PRO_DEF_FACADE(CopyableFacade, PRO_MAKE_DISPATCH_PACK(), pro::copyable_ptr_constraints);
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
static_assert(pro::proxiable<MockFunctionPtr, CopyableFacade>);
static_assert(!std::is_constructible_v<pro::proxy<CopyableFacade>, MockMovablePtr>);
static_assert(!std::is_assignable_v<pro::proxy<CopyableFacade>, MockMovablePtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<CopyableFacade>, MockCopyablePtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<CopyableFacade>, MockCopyablePtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<CopyableFacade>, MockCopyableSmallPtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<CopyableFacade>, MockCopyableSmallPtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<CopyableFacade>, MockTrivialPtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<CopyableFacade>, MockTrivialPtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<CopyableFacade>, MockFunctionPtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<CopyableFacade>, MockFunctionPtr>);
static_assert(sizeof(pro::proxy<CopyableFacade>) == 3 * sizeof(void*));  // VTABLE should not be embeded

PRO_DEF_FACADE(CopyableSmallFacade, PRO_MAKE_DISPATCH_PACK(), pro::proxiable_ptr_constraints{
    .max_size = sizeof(void*),
    .max_align = alignof(void*),
    .copyability = pro::constraint_level::nontrivial,
    .relocatability = pro::constraint_level::nothrow,
    .destructibility = pro::constraint_level::nothrow,
  });
static_assert(!pro::proxiable<MockMovablePtr, CopyableSmallFacade>);
static_assert(!pro::proxiable<MockCopyablePtr, CopyableSmallFacade>);
static_assert(pro::proxiable<MockCopyableSmallPtr, CopyableSmallFacade>);
static_assert(pro::proxiable<MockTrivialPtr, CopyableSmallFacade>);
static_assert(pro::proxiable<MockFunctionPtr, CopyableSmallFacade>);
static_assert(!std::is_constructible_v<pro::proxy<CopyableSmallFacade>, MockMovablePtr>);
static_assert(!std::is_assignable_v<pro::proxy<CopyableSmallFacade>, MockMovablePtr>);
static_assert(!std::is_constructible_v<pro::proxy<CopyableSmallFacade>, MockCopyablePtr>);
static_assert(!std::is_assignable_v<pro::proxy<CopyableSmallFacade>, MockCopyablePtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<CopyableSmallFacade>, MockCopyableSmallPtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<CopyableSmallFacade>, MockCopyableSmallPtr>);
static_assert(std::is_constructible_v<pro::proxy<CopyableSmallFacade>, MockTrivialPtr>);
static_assert(std::is_assignable_v<pro::proxy<CopyableSmallFacade>, MockTrivialPtr>);
static_assert(std::is_constructible_v<pro::proxy<CopyableSmallFacade>, MockFunctionPtr>);
static_assert(std::is_assignable_v<pro::proxy<CopyableSmallFacade>, MockFunctionPtr>);
static_assert(sizeof(pro::proxy<CopyableSmallFacade>) == 2 * sizeof(void*));  // VTABLE should not be embeded

PRO_DEF_FACADE(TrivialFacade, PRO_MAKE_DISPATCH_PACK(), pro::trivial_ptr_constraints);
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
static_assert(pro::proxiable<MockFunctionPtr, TrivialFacade>);
static_assert(!std::is_constructible_v<pro::proxy<TrivialFacade>, MockMovablePtr>);
static_assert(!std::is_assignable_v<pro::proxy<TrivialFacade>, MockMovablePtr>);
static_assert(!std::is_constructible_v<pro::proxy<TrivialFacade>, MockCopyablePtr>);
static_assert(!std::is_assignable_v<pro::proxy<TrivialFacade>, MockCopyablePtr>);
static_assert(!std::is_constructible_v<pro::proxy<TrivialFacade>, MockCopyableSmallPtr>);
static_assert(!std::is_assignable_v<pro::proxy<TrivialFacade>, MockCopyableSmallPtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<TrivialFacade>, MockTrivialPtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<TrivialFacade>, MockTrivialPtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<TrivialFacade>, MockFunctionPtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<TrivialFacade>, MockFunctionPtr>);
static_assert(sizeof(pro::proxy<TrivialFacade>) == 2 * sizeof(void*));  // VTABLE should be eliminated, but a placeholder is required

struct ReflectionOfSmallPtr {
  template <class P> requires(sizeof(P) <= sizeof(void*))
  constexpr ReflectionOfSmallPtr(std::in_place_type_t<P>) {}
};
PRO_DEF_FACADE(RelocatableFacadeWithReflection, PRO_MAKE_DISPATCH_PACK(), pro::relocatable_ptr_constraints, ReflectionOfSmallPtr);
static_assert(!pro::proxiable<MockMovablePtr, RelocatableFacadeWithReflection>);
static_assert(!pro::proxiable<MockCopyablePtr, RelocatableFacadeWithReflection>);
static_assert(pro::proxiable<MockCopyableSmallPtr, RelocatableFacadeWithReflection>);
static_assert(pro::proxiable<MockTrivialPtr, RelocatableFacadeWithReflection>);
static_assert(pro::proxiable<MockFunctionPtr, RelocatableFacadeWithReflection>);

struct RuntimeReflection {
  template <class P>
  explicit RuntimeReflection(std::in_place_type_t<P>) { throw std::runtime_error{"Not supported"}; }
};
PRO_DEF_FACADE(FacadeWithRuntimeReflection, PRO_MAKE_DISPATCH_PACK(), pro::relocatable_ptr_constraints, RuntimeReflection);
static_assert(!pro::proxiable<MockTrivialPtr, FacadeWithRuntimeReflection>);

struct FacadeWithTupleLikeDispatches {
  using dispatch_types = std::array<utils::spec::ToString, 1>;
  static constexpr auto constraints = pro::relocatable_ptr_constraints;
  using reflection_type = void;
};
static_assert(pro::facade<FacadeWithTupleLikeDispatches>);

struct BadFacade_MissingDispatchTypes {
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-const-variable"
#endif  // __clang__
  static constexpr auto constraints = pro::relocatable_ptr_constraints;
#ifdef __clang__
#pragma clang diagnostic pop
#endif  // __clang__
  using reflection_type = void;
};
static_assert(!pro::facade<BadFacade_MissingDispatchTypes>);

struct BadFacade_BadDispatchTypes {
  using dispatch_types = int;
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-const-variable"
#endif  // __clang__
  static constexpr auto constraints = pro::relocatable_ptr_constraints;
#ifdef __clang__
#pragma clang diagnostic pop
#endif  // __clang__
  using reflection_type = void;
};
static_assert(!pro::facade<BadFacade_BadDispatchTypes>);

struct BadFacade_MissingConstraints {
  using dispatch_types = std::tuple<>;
  using reflection_type = void;
};
static_assert(!pro::facade<BadFacade_MissingConstraints>);

struct BadFacade_BadConstraints_UnexpectedType {
  using dispatch_types = std::tuple<>;
  static constexpr auto constraints = 0;
  using reflection_type = void;
};
static_assert(!pro::facade<BadFacade_BadConstraints_UnexpectedType>);

struct BadFacade_BadConstraints_BadAlignment {
  using dispatch_types = std::tuple<>;
  static constexpr pro::proxiable_ptr_constraints constraints{
      .max_size = 6u,
      .max_align = 6u, // Should be a power of 2
      .copyability = pro::constraint_level::none,
      .relocatability = pro::constraint_level::nothrow,
      .destructibility = pro::constraint_level::nothrow,
  };
  using reflection_type = void;
};
static_assert(!pro::facade<BadFacade_BadConstraints_BadAlignment>);

struct BadFacade_BadConstraints_BadSize {
  using dispatch_types = std::tuple<>;
  static constexpr pro::proxiable_ptr_constraints constraints{
      .max_size = 6u, // Should be a multiple of max_alignment
      .max_align = 4u,
      .copyability = pro::constraint_level::none,
      .relocatability = pro::constraint_level::nothrow,
      .destructibility = pro::constraint_level::nothrow,
  };
  using reflection_type = void;
};
static_assert(!pro::facade<BadFacade_BadConstraints_BadSize>);

struct BadFacade_BadConstraints_NotConstant {
  using dispatch_types = std::tuple<>;
  static inline const auto constraints = pro::relocatable_ptr_constraints;
  using reflection_type = void;
};
static_assert(!pro::facade<BadFacade_BadConstraints_NotConstant>);

struct BadFacade_MissingReflectionType {
  using dispatch_types = std::tuple<>;
  static constexpr auto constraints = pro::relocatable_ptr_constraints;
};
static_assert(!pro::facade<BadFacade_MissingReflectionType>);

struct BadFacade_BadReflectionType {
  using dispatch_types = std::tuple<>;
  static constexpr auto constraints = pro::relocatable_ptr_constraints;
  using reflection_type = std::unique_ptr<int>;  // Probably constexpr, unknown until the evaluation of proxiablility
};
static_assert(pro::facade<BadFacade_BadReflectionType>);

}  // namespace
