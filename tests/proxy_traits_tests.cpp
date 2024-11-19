// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <array>
#include <stdexcept>
#include <type_traits>
#include "proxy.h"
#include "utils.h"

namespace proxy_traits_tests_details {

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

struct DefaultFacade : pro::facade_builder::build {};
static_assert(DefaultFacade::constraints.copyability == pro::constraint_level::none);
static_assert(DefaultFacade::constraints.relocatability == pro::constraint_level::nothrow);
static_assert(DefaultFacade::constraints.destructibility == pro::constraint_level::nothrow);
static_assert(DefaultFacade::constraints.max_size >= 2 * sizeof(void*));
static_assert(DefaultFacade::constraints.max_align >= sizeof(void*));
static_assert(std::is_same_v<DefaultFacade::convention_types, std::tuple<>>);
static_assert(std::is_same_v<DefaultFacade::reflection_types, std::tuple<>>);
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

static_assert(!std::is_copy_constructible_v<pro::proxy<DefaultFacade>>);
static_assert(!std::is_copy_assignable_v<pro::proxy<DefaultFacade>>);
static_assert(std::is_nothrow_move_constructible_v<pro::proxy<DefaultFacade>>);
static_assert(!std::is_trivially_move_constructible_v<pro::proxy<DefaultFacade>>);
static_assert(std::is_nothrow_move_assignable_v<pro::proxy<DefaultFacade>>);
static_assert(!std::is_trivially_move_assignable_v<pro::proxy<DefaultFacade>>);
static_assert(std::is_nothrow_destructible_v<pro::proxy<DefaultFacade>>);
static_assert(!std::is_trivially_destructible_v<pro::proxy<DefaultFacade>>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<DefaultFacade>, MockMovablePtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<DefaultFacade>, MockMovablePtr>);
static_assert(pro::proxiable<MockMovablePtr, DefaultFacade>);
static_assert(pro::proxiable<MockCopyablePtr, DefaultFacade>);
static_assert(pro::proxiable<MockCopyableSmallPtr, DefaultFacade>);
static_assert(pro::proxiable<MockTrivialPtr, DefaultFacade>);
static_assert(pro::proxiable<MockFunctionPtr, DefaultFacade>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<DefaultFacade>, MockMovablePtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<DefaultFacade>, MockMovablePtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<DefaultFacade>, MockCopyablePtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<DefaultFacade>, MockCopyablePtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<DefaultFacade>, MockCopyableSmallPtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<DefaultFacade>, MockCopyableSmallPtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<DefaultFacade>, MockTrivialPtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<DefaultFacade>, MockTrivialPtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<DefaultFacade>, MockFunctionPtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<DefaultFacade>, MockFunctionPtr>);
static_assert(sizeof(pro::proxy<DefaultFacade>) == 4 * sizeof(void*));  // VTABLE should be embeded

struct CopyableFacade : pro::facade_builder
    ::support_copy<pro::constraint_level::nontrivial>
    ::build {};
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

struct CopyableSmallFacade : pro::facade_builder
    ::restrict_layout<sizeof(void*), alignof(void*)>
    ::support_copy<pro::constraint_level::nontrivial>
    ::build {};
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

struct TrivialFacade : pro::facade_builder
    ::restrict_layout<sizeof(void*), alignof(void*)>
    ::support_copy<pro::constraint_level::trivial>
    ::support_relocation<pro::constraint_level::trivial>
    ::support_destruction<pro::constraint_level::trivial>
    ::build {};
static_assert(std::is_trivially_copy_constructible_v<pro::proxy<TrivialFacade>>);
static_assert(std::is_trivially_copy_assignable_v<pro::proxy<TrivialFacade>>);
static_assert(std::is_nothrow_move_constructible_v<pro::proxy<TrivialFacade>>);
static_assert(std::is_trivially_move_constructible_v<pro::proxy<TrivialFacade>>);
static_assert(std::is_nothrow_move_assignable_v<pro::proxy<TrivialFacade>>);
static_assert(std::is_trivially_move_assignable_v<pro::proxy<TrivialFacade>>);
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
struct RelocatableFacadeWithReflection : pro::facade_builder
    ::add_direct_reflection<ReflectionOfSmallPtr>
    ::build {};
static_assert(!pro::proxiable<MockMovablePtr, RelocatableFacadeWithReflection>);
static_assert(!pro::proxiable<MockCopyablePtr, RelocatableFacadeWithReflection>);
static_assert(pro::proxiable<MockCopyableSmallPtr, RelocatableFacadeWithReflection>);
static_assert(pro::proxiable<MockTrivialPtr, RelocatableFacadeWithReflection>);
static_assert(pro::proxiable<MockFunctionPtr, RelocatableFacadeWithReflection>);

struct RuntimeReflection {
  template <class P>
  explicit RuntimeReflection(std::in_place_type_t<P>) { throw std::runtime_error{"Not supported"}; }
};
struct FacadeWithRuntimeReflection : pro::facade_builder
    ::add_reflection<RuntimeReflection>
    ::build {};
static_assert(!pro::proxiable<MockTrivialPtr, FacadeWithRuntimeReflection>);

struct FacadeWithTupleLikeConventions {
  struct ToStringConvention {
    static constexpr bool is_direct = false;
    using dispatch_type = utils::spec::FreeToString;
    using overload_types = std::tuple<std::string()>;
  };
  using convention_types = std::array<ToStringConvention, 1>;
  using reflection_types = std::tuple<>;
  static constexpr auto constraints = pro::proxiable_ptr_constraints{
      .max_size = 2 * sizeof(void*),
      .max_align = alignof(void*),
      .copyability = pro::constraint_level::none,
      .relocatability = pro::constraint_level::nothrow,
      .destructibility = pro::constraint_level::nothrow,
  };
};
static_assert(pro::facade<FacadeWithTupleLikeConventions>);

struct BadFacade_MissingConventionTypes {
  using reflection_types = std::tuple<>;
  static constexpr auto constraints = pro::proxiable_ptr_constraints{
      .max_size = 2 * sizeof(void*),
      .max_align = alignof(void*),
      .copyability = pro::constraint_level::none,
      .relocatability = pro::constraint_level::nothrow,
      .destructibility = pro::constraint_level::nothrow,
  };
};
static_assert(!pro::facade<BadFacade_MissingConventionTypes>);

struct BadFacade_BadConventionTypes {
  using convention_types = int;
  using reflection_types = std::tuple<>;
  static constexpr auto constraints = pro::proxiable_ptr_constraints{
      .max_size = 2 * sizeof(void*),
      .max_align = alignof(void*),
      .copyability = pro::constraint_level::none,
      .relocatability = pro::constraint_level::nothrow,
      .destructibility = pro::constraint_level::nothrow,
  };
};
static_assert(!pro::facade<BadFacade_BadConventionTypes>);

struct BadFacade_MissingConstraints {
  using convention_types = std::tuple<>;
  using reflection_types = std::tuple<>;
};
static_assert(!pro::facade<BadFacade_MissingConstraints>);

struct BadFacade_BadConstraints_UnexpectedType {
  using convention_types = std::tuple<>;
  using reflection_types = std::tuple<>;
  static constexpr auto constraints = 0;
};
static_assert(!pro::facade<BadFacade_BadConstraints_UnexpectedType>);

struct BadFacade_BadConstraints_BadAlignment {
  using convention_types = std::tuple<>;
  using reflection_types = std::tuple<>;
  static constexpr pro::proxiable_ptr_constraints constraints{
      .max_size = 6u,
      .max_align = 6u, // Should be a power of 2
      .copyability = pro::constraint_level::none,
      .relocatability = pro::constraint_level::nothrow,
      .destructibility = pro::constraint_level::nothrow,
  };
};
static_assert(!pro::facade<BadFacade_BadConstraints_BadAlignment>);

struct BadFacade_BadConstraints_BadSize {
  using convention_types = std::tuple<>;
  using reflection_types = std::tuple<>;
  static constexpr pro::proxiable_ptr_constraints constraints{
      .max_size = 6u, // Should be a multiple of max_alignment
      .max_align = 4u,
      .copyability = pro::constraint_level::none,
      .relocatability = pro::constraint_level::nothrow,
      .destructibility = pro::constraint_level::nothrow,
  };
};
static_assert(!pro::facade<BadFacade_BadConstraints_BadSize>);

struct BadFacade_BadConstraints_NotConstant {
  using convention_types = std::tuple<>;
  using reflection_types = std::tuple<>;
  static const pro::proxiable_ptr_constraints constraints;
};
static_assert(!pro::facade<BadFacade_BadConstraints_NotConstant>);
const pro::proxiable_ptr_constraints BadFacade_BadConstraints_NotConstant::constraints{
    .max_size = 2 * sizeof(void*),
    .max_align = alignof(void*),
    .copyability = pro::constraint_level::none,
    .relocatability = pro::constraint_level::nothrow,
    .destructibility = pro::constraint_level::nothrow,
};

struct BadFacade_MissingReflectionTypes {
  using convention_types = std::tuple<>;
  static constexpr auto constraints = pro::proxiable_ptr_constraints{
      .max_size = 2 * sizeof(void*),
      .max_align = alignof(void*),
      .copyability = pro::constraint_level::none,
      .relocatability = pro::constraint_level::nothrow,
      .destructibility = pro::constraint_level::nothrow,
  };
};
static_assert(!pro::facade<BadFacade_MissingReflectionTypes>);

struct BadReflection {
  BadReflection() = delete;
};
struct BadFacade_BadReflectionType {
  using convention_types = std::tuple<>;
  using reflection_types = std::tuple<BadReflection>;
  static constexpr auto constraints = pro::proxiable_ptr_constraints{
      .max_size = 2 * sizeof(void*),
      .max_align = alignof(void*),
      .copyability = pro::constraint_level::none,
      .relocatability = pro::constraint_level::nothrow,
      .destructibility = pro::constraint_level::nothrow,
  };
};
static_assert(pro::facade<BadFacade_BadReflectionType>);

PRO_DEF_MEM_DISPATCH(MemFoo, Foo);
PRO_DEF_MEM_DISPATCH(MemBar, Bar);
struct BigFacade : pro::facade_builder
    ::add_convention<MemFoo, void(), void(int)>
    ::add_convention<MemBar, void(), void(int)>
    ::add_direct_convention<MemFoo, void(), void(int)>
    ::add_direct_convention<MemBar, void(), void(int)>
    ::build {};
static_assert(sizeof(pro::proxy<BigFacade>) == 3 * sizeof(void*));  // Accessors should not add paddings

struct FacadeWithSizeOfNonPowerOfTwo : pro::facade_builder
    ::restrict_layout<6u>
    ::build {};
static_assert(pro::facade<FacadeWithSizeOfNonPowerOfTwo>);
static_assert(FacadeWithSizeOfNonPowerOfTwo::constraints.max_size == 6u);
static_assert(FacadeWithSizeOfNonPowerOfTwo::constraints.max_align == 2u);

}  // namespace proxy_traits_tests_details
