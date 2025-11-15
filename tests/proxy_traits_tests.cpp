// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "utils.h"
#include <array>
#include <proxy/proxy.h>
#include <stdexcept>
#include <type_traits>

namespace proxy_traits_tests_details {

template <bool kNothrowRelocatable, bool kCopyable, bool kTrivial,
          std::size_t kSize, std::size_t kAlignment>
struct MockPtr {
  using element_type = MockPtr;

  MockPtr() = default;
  MockPtr(int) noexcept {}
  MockPtr(const MockPtr&)
    requires(kCopyable && !kTrivial)
  {}
  MockPtr(const MockPtr&) noexcept
    requires(kTrivial)
  = default;
  MockPtr(MockPtr&&) noexcept
    requires(kNothrowRelocatable && !kTrivial)
  {}
  MockPtr(MockPtr&&) noexcept
    requires(kTrivial)
  = default;
  const MockPtr* operator->() const noexcept { return this; }

  alignas(kAlignment) char dummy_[kSize];
};
using MockMovablePtr =
    MockPtr<true, false, false, sizeof(void*) * 2, alignof(void*)>;
using MockCopyablePtr =
    MockPtr<true, true, false, sizeof(void*) * 2, alignof(void*)>;
using MockCopyableSmallPtr =
    MockPtr<true, true, false, sizeof(void*), alignof(void*)>;
using MockTrivialPtr = MockPtr<true, true, true, sizeof(void*), alignof(void*)>;
using MockFunctionPtr = void (*)();

} // namespace proxy_traits_tests_details

namespace pro {

template <bool kNothrowRelocatable, bool kCopyable, bool kTrivial,
          std::size_t kSize, std::size_t kAlignment>
struct is_bitwise_trivially_relocatable<proxy_traits_tests_details::MockPtr<
    kNothrowRelocatable, kCopyable, kTrivial, kSize, kAlignment>>
    : std::true_type {};

} // namespace pro

namespace proxy_traits_tests_details {

struct DefaultFacade : pro::facade_builder::build {};
static_assert(
    std::is_same_v<pro::proxy<DefaultFacade>::facade_type, DefaultFacade>);
static_assert(DefaultFacade::copyability == pro::constraint_level::none);
static_assert(DefaultFacade::relocatability == pro::constraint_level::trivial);
static_assert(DefaultFacade::destructibility == pro::constraint_level::nothrow);
static_assert(DefaultFacade::max_size >= 2 * sizeof(void*));
static_assert(DefaultFacade::max_align >= sizeof(void*));
static_assert(std::is_same_v<DefaultFacade::convention_types, std::tuple<>>);
static_assert(std::is_same_v<DefaultFacade::reflection_types, std::tuple<>>);
static_assert(
    std::is_nothrow_default_constructible_v<pro::proxy<DefaultFacade>>);
static_assert(
    !std::is_trivially_default_constructible_v<pro::proxy<DefaultFacade>>);
static_assert(
    std::is_nothrow_constructible_v<pro::proxy<DefaultFacade>, std::nullptr_t>);
static_assert(
    std::is_nothrow_assignable_v<pro::proxy<DefaultFacade>, std::nullptr_t>);
static_assert(
    std::is_nothrow_constructible_v<pro::proxy<DefaultFacade>,
                                    std::in_place_type_t<MockMovablePtr>, int>);
static_assert(
    std::is_nothrow_constructible_v<
        pro::proxy<DefaultFacade>, std::in_place_type_t<MockCopyablePtr>, int>);
static_assert(std::is_nothrow_constructible_v<
              pro::proxy<DefaultFacade>,
              std::in_place_type_t<MockCopyableSmallPtr>, int>);
static_assert(
    std::is_nothrow_constructible_v<pro::proxy<DefaultFacade>,
                                    std::in_place_type_t<MockTrivialPtr>, int>);
static_assert(std::is_nothrow_constructible_v<
              pro::proxy<DefaultFacade>, std::in_place_type_t<MockFunctionPtr>,
              MockFunctionPtr>);
static_assert(sizeof(pro::proxy<DefaultFacade>) ==
              3 * sizeof(void*)); // VTABLE should be embeded

static_assert(!std::is_copy_constructible_v<pro::proxy<DefaultFacade>>);
static_assert(!std::is_copy_assignable_v<pro::proxy<DefaultFacade>>);
static_assert(std::is_nothrow_move_constructible_v<pro::proxy<DefaultFacade>>);
static_assert(
    !std::is_trivially_move_constructible_v<pro::proxy<DefaultFacade>>);
static_assert(std::is_nothrow_move_assignable_v<pro::proxy<DefaultFacade>>);
static_assert(!std::is_trivially_move_assignable_v<pro::proxy<DefaultFacade>>);
static_assert(std::is_nothrow_destructible_v<pro::proxy<DefaultFacade>>);
static_assert(!std::is_trivially_destructible_v<pro::proxy<DefaultFacade>>);
static_assert(
    std::is_nothrow_constructible_v<pro::proxy<DefaultFacade>, MockMovablePtr>);
static_assert(
    std::is_nothrow_assignable_v<pro::proxy<DefaultFacade>, MockMovablePtr>);
static_assert(pro::proxiable<MockMovablePtr, DefaultFacade>);
static_assert(pro::proxiable<MockCopyablePtr, DefaultFacade>);
static_assert(pro::proxiable<MockCopyableSmallPtr, DefaultFacade>);
static_assert(pro::proxiable<MockTrivialPtr, DefaultFacade>);
static_assert(pro::proxiable<MockFunctionPtr, DefaultFacade>);
static_assert(
    std::is_nothrow_constructible_v<pro::proxy<DefaultFacade>, MockMovablePtr>);
static_assert(
    std::is_nothrow_assignable_v<pro::proxy<DefaultFacade>, MockMovablePtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<DefaultFacade>,
                                              MockCopyablePtr>);
static_assert(
    std::is_nothrow_assignable_v<pro::proxy<DefaultFacade>, MockCopyablePtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<DefaultFacade>,
                                              MockCopyableSmallPtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<DefaultFacade>,
                                           MockCopyableSmallPtr>);
static_assert(
    std::is_nothrow_constructible_v<pro::proxy<DefaultFacade>, MockTrivialPtr>);
static_assert(
    std::is_nothrow_assignable_v<pro::proxy<DefaultFacade>, MockTrivialPtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<DefaultFacade>,
                                              MockFunctionPtr>);
static_assert(
    std::is_nothrow_assignable_v<pro::proxy<DefaultFacade>, MockFunctionPtr>);

struct CopyableFacade : pro::facade_builder                               //
                        ::support_copy<pro::constraint_level::nontrivial> //
                        ::build {};
static_assert(std::is_copy_constructible_v<pro::proxy<CopyableFacade>>);
static_assert(
    !std::is_nothrow_copy_constructible_v<pro::proxy<CopyableFacade>>);
static_assert(std::is_copy_assignable_v<pro::proxy<CopyableFacade>>);
static_assert(!std::is_nothrow_copy_assignable_v<pro::proxy<CopyableFacade>>);
static_assert(std::is_nothrow_move_constructible_v<pro::proxy<CopyableFacade>>);
static_assert(
    !std::is_trivially_move_constructible_v<pro::proxy<CopyableFacade>>);
static_assert(std::is_nothrow_move_assignable_v<pro::proxy<CopyableFacade>>);
static_assert(!std::is_trivially_move_assignable_v<pro::proxy<CopyableFacade>>);
static_assert(std::is_nothrow_destructible_v<pro::proxy<CopyableFacade>>);
static_assert(!std::is_trivially_destructible_v<pro::proxy<CopyableFacade>>);
static_assert(!pro::proxiable<MockMovablePtr, CopyableFacade>);
static_assert(pro::proxiable<MockCopyablePtr, CopyableFacade>);
static_assert(pro::proxiable<MockCopyableSmallPtr, CopyableFacade>);
static_assert(pro::proxiable<MockTrivialPtr, CopyableFacade>);
static_assert(pro::proxiable<MockFunctionPtr, CopyableFacade>);
static_assert(
    std::is_constructible_v<pro::proxy<CopyableFacade>, MockMovablePtr>);
static_assert(std::is_assignable_v<pro::proxy<CopyableFacade>, MockMovablePtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<CopyableFacade>,
                                              MockCopyablePtr>);
static_assert(
    std::is_nothrow_assignable_v<pro::proxy<CopyableFacade>, MockCopyablePtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<CopyableFacade>,
                                              MockCopyableSmallPtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<CopyableFacade>,
                                           MockCopyableSmallPtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<CopyableFacade>,
                                              MockTrivialPtr>);
static_assert(
    std::is_nothrow_assignable_v<pro::proxy<CopyableFacade>, MockTrivialPtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<CopyableFacade>,
                                              MockFunctionPtr>);
static_assert(
    std::is_nothrow_assignable_v<pro::proxy<CopyableFacade>, MockFunctionPtr>);
static_assert(sizeof(pro::proxy<CopyableFacade>) ==
              4 * sizeof(void*)); // VTABLE should be embeded

struct CopyableSmallFacade
    : pro::facade_builder                               //
      ::add_skill<pro::skills::slim>                    //
      ::support_copy<pro::constraint_level::nontrivial> //
      ::build {};
static_assert(!pro::proxiable<MockMovablePtr, CopyableSmallFacade>);
static_assert(!pro::proxiable<MockCopyablePtr, CopyableSmallFacade>);
static_assert(pro::proxiable<MockCopyableSmallPtr, CopyableSmallFacade>);
static_assert(pro::proxiable<MockTrivialPtr, CopyableSmallFacade>);
static_assert(pro::proxiable<MockFunctionPtr, CopyableSmallFacade>);
static_assert(
    std::is_constructible_v<pro::proxy<CopyableSmallFacade>, MockMovablePtr>);
static_assert(
    std::is_assignable_v<pro::proxy<CopyableSmallFacade>, MockMovablePtr>);
static_assert(
    std::is_constructible_v<pro::proxy<CopyableSmallFacade>, MockCopyablePtr>);
static_assert(
    std::is_assignable_v<pro::proxy<CopyableSmallFacade>, MockCopyablePtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<CopyableSmallFacade>,
                                              MockCopyableSmallPtr>);
static_assert(std::is_nothrow_assignable_v<pro::proxy<CopyableSmallFacade>,
                                           MockCopyableSmallPtr>);
static_assert(
    std::is_constructible_v<pro::proxy<CopyableSmallFacade>, MockTrivialPtr>);
static_assert(
    std::is_assignable_v<pro::proxy<CopyableSmallFacade>, MockTrivialPtr>);
static_assert(
    std::is_constructible_v<pro::proxy<CopyableSmallFacade>, MockFunctionPtr>);
static_assert(
    std::is_assignable_v<pro::proxy<CopyableSmallFacade>, MockFunctionPtr>);
static_assert(sizeof(pro::proxy<CopyableSmallFacade>) ==
              3 * sizeof(void*)); // VTABLE should be embeded

struct TrivialFacade : pro::facade_builder                                   //
                       ::restrict_layout<sizeof(void*), alignof(void*)>      //
                       ::support_copy<pro::constraint_level::trivial>        //
                       ::support_relocation<pro::constraint_level::trivial>  //
                       ::support_destruction<pro::constraint_level::trivial> //
                       ::build {};
static_assert(
    std::is_trivially_copy_constructible_v<pro::proxy<TrivialFacade>>);
static_assert(std::is_trivially_copy_assignable_v<pro::proxy<TrivialFacade>>);
static_assert(std::is_nothrow_move_constructible_v<pro::proxy<TrivialFacade>>);
static_assert(
    std::is_trivially_move_constructible_v<pro::proxy<TrivialFacade>>);
static_assert(std::is_nothrow_move_assignable_v<pro::proxy<TrivialFacade>>);
static_assert(std::is_trivially_move_assignable_v<pro::proxy<TrivialFacade>>);
static_assert(std::is_trivially_destructible_v<pro::proxy<TrivialFacade>>);
static_assert(!pro::proxiable<MockMovablePtr, TrivialFacade>);
static_assert(!pro::proxiable<MockCopyablePtr, TrivialFacade>);
static_assert(!pro::proxiable<MockCopyableSmallPtr, TrivialFacade>);
static_assert(pro::proxiable<MockTrivialPtr, TrivialFacade>);
static_assert(pro::proxiable<MockFunctionPtr, TrivialFacade>);
static_assert(
    std::is_constructible_v<pro::proxy<TrivialFacade>, MockMovablePtr>);
static_assert(std::is_assignable_v<pro::proxy<TrivialFacade>, MockMovablePtr>);
static_assert(
    std::is_constructible_v<pro::proxy<TrivialFacade>, MockCopyablePtr>);
static_assert(std::is_assignable_v<pro::proxy<TrivialFacade>, MockCopyablePtr>);
static_assert(
    std::is_constructible_v<pro::proxy<TrivialFacade>, MockCopyableSmallPtr>);
static_assert(
    std::is_assignable_v<pro::proxy<TrivialFacade>, MockCopyableSmallPtr>);
static_assert(
    std::is_nothrow_constructible_v<pro::proxy<TrivialFacade>, MockTrivialPtr>);
static_assert(
    std::is_nothrow_assignable_v<pro::proxy<TrivialFacade>, MockTrivialPtr>);
static_assert(std::is_nothrow_constructible_v<pro::proxy<TrivialFacade>,
                                              MockFunctionPtr>);
static_assert(
    std::is_nothrow_assignable_v<pro::proxy<TrivialFacade>, MockFunctionPtr>);
static_assert(sizeof(pro::proxy<TrivialFacade>) ==
              2 * sizeof(void*)); // VTABLE should be eliminated, but a
                                  // placeholder is required

struct ReflectionOfSmallPtr {
  template <class P>
    requires(sizeof(P) <= sizeof(void*))
  constexpr ReflectionOfSmallPtr(std::in_place_type_t<P>) {}
};
struct RelocatableFacadeWithReflection
    : pro::facade_builder                           //
      ::add_direct_reflection<ReflectionOfSmallPtr> //
      ::build {};
static_assert(!pro::proxiable<MockMovablePtr, RelocatableFacadeWithReflection>);
static_assert(
    !pro::proxiable<MockCopyablePtr, RelocatableFacadeWithReflection>);
static_assert(
    pro::proxiable<MockCopyableSmallPtr, RelocatableFacadeWithReflection>);
static_assert(pro::proxiable<MockTrivialPtr, RelocatableFacadeWithReflection>);
static_assert(pro::proxiable<MockFunctionPtr, RelocatableFacadeWithReflection>);

struct RuntimeReflection {
  template <class P>
  explicit RuntimeReflection(std::in_place_type_t<P>) {
    throw std::runtime_error{"Not supported"};
  }
};
struct FacadeWithRuntimeReflection : pro::facade_builder                 //
                                     ::add_reflection<RuntimeReflection> //
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
  static constexpr std::size_t max_size = 2 * sizeof(void*);
  static constexpr std::size_t max_align = alignof(void*);
  static constexpr auto copyability = pro::constraint_level::none;
  static constexpr auto relocatability = pro::constraint_level::nothrow;
  static constexpr auto destructibility = pro::constraint_level::nothrow;
};
static_assert(pro::facade<FacadeWithTupleLikeConventions>);

struct BadFacade_MissingConventionTypes {
  using reflection_types = std::tuple<>;
  static constexpr std::size_t max_size = 2 * sizeof(void*);
  static constexpr std::size_t max_align = alignof(void*);
  static constexpr auto copyability = pro::constraint_level::none;
  static constexpr auto relocatability = pro::constraint_level::nothrow;
  static constexpr auto destructibility = pro::constraint_level::nothrow;
};
static_assert(!pro::facade<BadFacade_MissingConventionTypes>);

struct BadFacade_BadConventionTypes {
  using convention_types = int;
  using reflection_types = std::tuple<>;
  static constexpr std::size_t max_size = 2 * sizeof(void*);
  static constexpr std::size_t max_align = alignof(void*);
  static constexpr auto copyability = pro::constraint_level::none;
  static constexpr auto relocatability = pro::constraint_level::nothrow;
  static constexpr auto destructibility = pro::constraint_level::nothrow;
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

// Well-formed facade_impl specialization
static_assert(
    pro::facade<pro::details::facade_impl<
        std::tuple<>, std::tuple<>, 8, 4, pro::constraint_level::none,
        pro::constraint_level::trivial, pro::constraint_level::nothrow>>);

// Bad size (max_size should be positive)
static_assert(
    !pro::facade<pro::details::facade_impl<
        std::tuple<>, std::tuple<>, 0, 4, pro::constraint_level::none,
        pro::constraint_level::trivial, pro::constraint_level::nothrow>>);

// Bad size (max_size should be a multiple of max_align)
static_assert(
    !pro::facade<pro::details::facade_impl<
        std::tuple<>, std::tuple<>, 10, 4, pro::constraint_level::none,
        pro::constraint_level::trivial, pro::constraint_level::nothrow>>);

// Bad alignment (max_align should be a power of 2)
static_assert(
    !pro::facade<pro::details::facade_impl<
        std::tuple<>, std::tuple<>, 6, 6, pro::constraint_level::none,
        pro::constraint_level::trivial, pro::constraint_level::nothrow>>);

// Bad copyability (less than constraint_level::none)
static_assert(
    !pro::facade<pro::details::facade_impl<
        std::tuple<>, std::tuple<>, 8, 4, (pro::constraint_level)-1,
        pro::constraint_level::trivial, pro::constraint_level::nothrow>>);

// Bad copyability (greater than constraint_level::trivial)
static_assert(
    !pro::facade<pro::details::facade_impl<
        std::tuple<>, std::tuple<>, 8, 4, (pro::constraint_level)100,
        pro::constraint_level::trivial, pro::constraint_level::nothrow>>);

// Bad relocatability (less than constraint_level::none)
static_assert(!pro::facade<pro::details::facade_impl<
                  std::tuple<>, std::tuple<>, 8, 4, pro::constraint_level::none,
                  (pro::constraint_level)-1, pro::constraint_level::nothrow>>);

// Bad relocatability (greater than constraint_level::trivial)
static_assert(!pro::facade<pro::details::facade_impl<
                  std::tuple<>, std::tuple<>, 8, 4, pro::constraint_level::none,
                  (pro::constraint_level)100, pro::constraint_level::nothrow>>);

// Bad destructibility (less than constraint_level::none)
static_assert(!pro::facade<pro::details::facade_impl<
                  std::tuple<>, std::tuple<>, 8, 4, pro::constraint_level::none,
                  pro::constraint_level::trivial, (pro::constraint_level)-1>>);

// Bad destructibility (greater than constraint_level::trivial)
static_assert(!pro::facade<pro::details::facade_impl<
                  std::tuple<>, std::tuple<>, 8, 4, pro::constraint_level::none,
                  pro::constraint_level::trivial, (pro::constraint_level)100>>);

struct BadFacade_BadConstraints_NotConstant {
  using convention_types = std::tuple<>;
  using reflection_types = std::tuple<>;
  static const std::size_t max_size;
  static constexpr std::size_t max_align = alignof(void*);
  static constexpr auto copyability = pro::constraint_level::none;
  static constexpr auto relocatability = pro::constraint_level::nothrow;
  static constexpr auto destructibility = pro::constraint_level::nothrow;
};
static_assert(!pro::facade<BadFacade_BadConstraints_NotConstant>);
const std::size_t BadFacade_BadConstraints_NotConstant::max_size =
    2 * sizeof(void*);
struct BadFacade_MissingReflectionTypes {
  using convention_types = std::tuple<>;
  static constexpr std::size_t max_size = 2 * sizeof(void*);
  static constexpr std::size_t max_align = alignof(void*);
  static constexpr auto copyability = pro::constraint_level::none;
  static constexpr auto relocatability = pro::constraint_level::nothrow;
  static constexpr auto destructibility = pro::constraint_level::nothrow;
};
static_assert(!pro::facade<BadFacade_MissingReflectionTypes>);

struct BadReflection {
  BadReflection() = delete;
};
struct BadFacade_BadReflectionType {
  using convention_types = std::tuple<>;
  using reflection_types = std::tuple<BadReflection>;
  static constexpr std::size_t max_size = 2 * sizeof(void*);
  static constexpr std::size_t max_align = alignof(void*);
  static constexpr auto copyability = pro::constraint_level::none;
  static constexpr auto relocatability = pro::constraint_level::nothrow;
  static constexpr auto destructibility = pro::constraint_level::nothrow;
};
static_assert(!pro::facade<BadFacade_BadReflectionType>);

PRO_DEF_MEM_DISPATCH(MemFoo, Foo);
PRO_DEF_MEM_DISPATCH(MemBar, Bar);
struct BigFacade : pro::facade_builder                                //
                   ::add_convention<MemFoo, void(), void(int)>        //
                   ::add_convention<MemBar, void(), void(int)>        //
                   ::add_direct_convention<MemFoo, void(), void(int)> //
                   ::add_direct_convention<MemBar, void(), void(int)> //
                   ::build {};
static_assert(sizeof(pro::proxy<BigFacade>) ==
              3 * sizeof(void*)); // Accessors should not add paddings

struct FacadeWithSizeOfNonPowerOfTwo : pro::facade_builder   //
                                       ::restrict_layout<6u> //
                                       ::build {};
static_assert(pro::facade<FacadeWithSizeOfNonPowerOfTwo>);
static_assert(FacadeWithSizeOfNonPowerOfTwo::max_size == 6u);
static_assert(FacadeWithSizeOfNonPowerOfTwo::max_align == 2u);

template <std::size_t Size, std::size_t Align>
concept IsFacadeBuilderWellFormedWithGivenLayout =
    requires { typename pro::facade_builder::restrict_layout<Size, Align>; };
static_assert(IsFacadeBuilderWellFormedWithGivenLayout<6u, 1u>);
static_assert(!IsFacadeBuilderWellFormedWithGivenLayout<6u, 3u>);
static_assert(!IsFacadeBuilderWellFormedWithGivenLayout<1u, 2u>);

template <pro::constraint_level CL>
concept IsFacadeBuilderWellFormedWithGivenCopyability =
    requires { typename pro::facade_builder::support_copy<CL>; };
static_assert(
    IsFacadeBuilderWellFormedWithGivenCopyability<pro::constraint_level::none>);
static_assert(
    !IsFacadeBuilderWellFormedWithGivenCopyability<(pro::constraint_level)-1>);
static_assert(
    !IsFacadeBuilderWellFormedWithGivenCopyability<(pro::constraint_level)100>);

template <pro::constraint_level CL>
concept IsFacadeBuilderWellFormedWithGivenRelocatability =
    requires { typename pro::facade_builder::support_relocation<CL>; };
static_assert(IsFacadeBuilderWellFormedWithGivenRelocatability<
              pro::constraint_level::none>);
static_assert(!IsFacadeBuilderWellFormedWithGivenRelocatability<
              (pro::constraint_level)-1>);
static_assert(!IsFacadeBuilderWellFormedWithGivenRelocatability<
              (pro::constraint_level)100>);

template <pro::constraint_level CL>
concept IsFacadeBuilderWellFormedWithGivenDestructibility =
    requires { typename pro::facade_builder::support_destruction<CL>; };
static_assert(IsFacadeBuilderWellFormedWithGivenDestructibility<
              pro::constraint_level::none>);
static_assert(!IsFacadeBuilderWellFormedWithGivenDestructibility<
              (pro::constraint_level)-1>);
static_assert(!IsFacadeBuilderWellFormedWithGivenDestructibility<
              (pro::constraint_level)100>);

static_assert(!std::is_default_constructible_v<
              pro::proxy_indirect_accessor<DefaultFacade>>);
static_assert(
    !std::is_copy_constructible_v<pro::proxy_indirect_accessor<DefaultFacade>>);
static_assert(
    !std::is_move_constructible_v<pro::proxy_indirect_accessor<DefaultFacade>>);
static_assert(
    !std::is_copy_assignable_v<pro::proxy_indirect_accessor<DefaultFacade>>);
static_assert(
    !std::is_move_assignable_v<pro::proxy_indirect_accessor<DefaultFacade>>);

// proxy shall not be constructible from an arbitrary class template
// instantiation. See https://github.com/microsoft/proxy/issues/366
template <class T>
struct ProxyWrapperTemplate {
  explicit ProxyWrapperTemplate(pro::proxy<DefaultFacade> p)
      : p_(std::move(p)) {}

  pro::proxy<DefaultFacade> p_;
};
static_assert(!pro::proxiable<int, DefaultFacade>);
static_assert(!std::is_constructible_v<pro::proxy<DefaultFacade>,
                                       ProxyWrapperTemplate<void>>);
static_assert(std::is_move_constructible_v<ProxyWrapperTemplate<void>>);

} // namespace proxy_traits_tests_details
