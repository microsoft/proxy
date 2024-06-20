// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef _MSFT_PROXY_
#define _MSFT_PROXY_

#include <cstddef>
#include <bit>
#include <concepts>
#include <initializer_list>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

#if __has_cpp_attribute(msvc::no_unique_address)
#define ___PRO_NO_UNIQUE_ADDRESS_ATTRIBUTE msvc::no_unique_address
#elif __has_cpp_attribute(no_unique_address)
#define ___PRO_NO_UNIQUE_ADDRESS_ATTRIBUTE no_unique_address
#else
#error "Proxy requires C++20 attribute no_unique_address"
#endif

#define ___PRO_DIRECT_FUNC_IMPL(...) \
    noexcept(noexcept(__VA_ARGS__)) requires(requires { __VA_ARGS__; }) \
    { return __VA_ARGS__; }

namespace pro {

enum class constraint_level { none, nontrivial, nothrow, trivial };

struct proxiable_ptr_constraints {
  std::size_t max_size;
  std::size_t max_align;
  constraint_level copyability;
  constraint_level relocatability;
  constraint_level destructibility;
};

template <class F> class proxy;

namespace details {

template <bool A>
struct conditional_traits { static constexpr bool applicable = A; };
using applicable_traits = conditional_traits<true>;
using inapplicable_traits = conditional_traits<false>;

template <class T, class...> struct lazy_eval_traits : std::type_identity<T> {};
template <class U, class... T>
using lazy_eval_t = typename lazy_eval_traits<U, T...>::type;

enum class qualifier_type { lv, const_lv, rv, const_rv };

template <class T> struct qualifier_of_traits;
template <class T>
struct qualifier_of_traits<T&>
    : std::integral_constant<qualifier_type, qualifier_type::lv> {};
template <class T>
struct qualifier_of_traits<const T&>
    : std::integral_constant<qualifier_type, qualifier_type::const_lv> {};
template <class T>
struct qualifier_of_traits<T&&>
    : std::integral_constant<qualifier_type, qualifier_type::rv> {};
template <class T>
struct qualifier_of_traits<const T&&>
    : std::integral_constant<qualifier_type, qualifier_type::const_rv> {};
template <class T>
constexpr qualifier_type qualifier_of_v = qualifier_of_traits<T>::value;

template <class T, qualifier_type Q> struct add_qualifier_traits;
template <class T>
struct add_qualifier_traits<T, qualifier_type::lv> : std::type_identity<T&> {};
template <class T>
struct add_qualifier_traits<T, qualifier_type::const_lv>
    : std::type_identity<const T&> {};
template <class T>
struct add_qualifier_traits<T, qualifier_type::rv> : std::type_identity<T&&> {};
template <class T>
struct add_qualifier_traits<T, qualifier_type::const_rv>
    : std::type_identity<const T&&> {};
template <class T, qualifier_type Q>
using add_qualifier_t = typename add_qualifier_traits<T, Q>::type;

template <template <class, class> class R, class O, class... Is>
struct recursive_reduction : std::type_identity<O> {};
template <template <class, class> class R, class O, class... Is>
using recursive_reduction_t = typename recursive_reduction<R, O, Is...>::type;
template <template <class, class> class R, class O, class I, class... Is>
struct recursive_reduction<R, O, I, Is...>
    { using type = recursive_reduction_t<R, R<O, I>, Is...>; };

template <template <class> class T, class... Is> struct first_applicable {};
template <template <class> class T, class I, class... Is>
    requires(T<I>::applicable)
struct first_applicable<T, I, Is...> : std::type_identity<I> {};
template <template <class> class T, class I, class... Is>
struct first_applicable<T, I, Is...> : first_applicable<T, Is...> {};
template <template <class> class T, class... Is>
using first_applicable_t = typename first_applicable<T, Is...>::type;

template <class Expr>
consteval bool is_consteval(Expr)
    { return requires { typename std::bool_constant<(Expr{}(), false)>; }; }

template <class T, std::size_t I>
concept has_tuple_element = requires { typename std::tuple_element_t<I, T>; };
template <class T>
consteval bool is_tuple_like_well_formed() {
  if constexpr (requires { { std::tuple_size<T>::value } ->
      std::same_as<const std::size_t&>; }) {
    if constexpr (is_consteval([] { return std::tuple_size<T>::value; })) {
      return []<std::size_t... I>(std::index_sequence<I...>) {
        return (has_tuple_element<T, I> && ...);
      }(std::make_index_sequence<std::tuple_size_v<T>>{});
    }
  }
  return false;
}

template <template <class...> class T, class TL, class Is, class... Args>
struct instantiated_traits;
template <template <class...> class T, class TL, std::size_t... Is,
    class... Args>
struct instantiated_traits<T, TL, std::index_sequence<Is...>, Args...>
    { using type = T<Args..., std::tuple_element_t<Is, TL>...>; };
template <template <class...> class T, class TL, class... Args>
using instantiated_t = typename instantiated_traits<
    T, TL, std::make_index_sequence<std::tuple_size_v<TL>>, Args...>::type;

template <class T>
consteval bool has_copyability(constraint_level level) {
  switch (level) {
    case constraint_level::trivial:
      return std::is_trivially_copy_constructible_v<T>;
    case constraint_level::nothrow:
      return std::is_nothrow_copy_constructible_v<T>;
    case constraint_level::nontrivial: return std::is_copy_constructible_v<T>;
    case constraint_level::none: return true;
    default: return false;
  }
}
template <class T>
consteval bool has_relocatability(constraint_level level) {
  switch (level) {
    case constraint_level::trivial:
      return std::is_trivially_move_constructible_v<T> &&
          std::is_trivially_destructible_v<T>;
    case constraint_level::nothrow:
      return std::is_nothrow_move_constructible_v<T> &&
          std::is_nothrow_destructible_v<T>;
    case constraint_level::nontrivial:
      return std::is_move_constructible_v<T> && std::is_destructible_v<T>;
    case constraint_level::none: return true;
    default: return false;
  }
}
template <class T>
consteval bool has_destructibility(constraint_level level) {
  switch (level) {
    case constraint_level::trivial: return std::is_trivially_destructible_v<T>;
    case constraint_level::nothrow: return std::is_nothrow_destructible_v<T>;
    case constraint_level::nontrivial: return std::is_destructible_v<T>;
    case constraint_level::none: return true;
    default: return false;
  }
}

// As per std::to_address() wording in [pointer.conversion]
template <class P> struct ptr_traits : inapplicable_traits {};
template <class P>
    requires(requires(const P p) { std::pointer_traits<P>::to_address(p); } ||
        requires(const P p) { p.operator->(); })
struct ptr_traits<P> : applicable_traits {
  static auto& dereference(const P& p) noexcept { return *std::to_address(p); }
  using target_type = std::remove_pointer_t<
      decltype(std::to_address(std::declval<const P&>()))>;
};
template <class T>
struct ptr_traits<T*> : applicable_traits {
  static T& dereference(T* p) noexcept { return *p; }
  using target_type = T;
};

template <class D, bool NE, class R, class... Args>
constexpr bool invocable_dispatch = std::conditional_t<
    NE, std::is_nothrow_invocable_r<R, D, Args...>,
    std::is_invocable_r<R, D, Args...>>::value;
template <class D, class P, bool NE, class R, class... Args>
concept invocable_indirect_dispatch_ptr = invocable_dispatch<
    D, NE, R, typename ptr_traits<P>::target_type&, Args...> ||
    invocable_dispatch<D, NE, R, std::nullptr_t, Args...>;
template <class D, class P, qualifier_type Q, bool NE, class R, class... Args>
concept invocable_direct_dispatch_ptr = invocable_dispatch<
    D, NE, R, add_qualifier_t<P, Q>, Args...> ||
    invocable_dispatch<D, NE, R, std::nullptr_t, Args...>;

template <bool NE, class R, class... Args>
using func_ptr_t = std::conditional_t<
    NE, R (*)(Args...) noexcept, R (*)(Args...)>;

template <class D, class R, class... Args>
R invoke_dispatch(Args&&... args) {
  if constexpr (std::is_void_v<R>) {
    D{}(std::forward<Args>(args)...);
  } else {
    return D{}(std::forward<Args>(args)...);
  }
}
template <class P, class D, class R, class... Args>
R indirect_conv_dispatcher(const std::byte& self, Args... args)
    noexcept(invocable_dispatch<
        D, true, R, typename ptr_traits<P>::target_type&, Args...>) {
  return invoke_dispatch<D, R>(ptr_traits<P>::dereference(*std::launder(
      reinterpret_cast<const P*>(&self))), std::forward<Args>(args)...);
}
template <class P, class D, qualifier_type Q, class R, class... Args>
R direct_conv_dispatcher(add_qualifier_t<std::byte, Q> self, Args... args)
    noexcept(invocable_dispatch<
        D, true, R, typename ptr_traits<P>::target_type&, Args...>) {
  using Ptr = add_qualifier_t<P, Q>;
  return invoke_dispatch<D, R>(std::forward<Ptr>(*std::launder(reinterpret_cast<
      std::remove_reference_t<Ptr>*>(&self))), std::forward<Args>(args)...);
}
template <class D, qualifier_type Q, class R, class... Args>
R default_conv_dispatcher(add_qualifier_t<std::byte, Q>, Args... args)
    noexcept(invocable_dispatch<D, true, R, std::nullptr_t, Args...>)
    { return invoke_dispatch<D, R>(nullptr, std::forward<Args>(args)...); }
template <class P>
void copying_dispatcher(std::byte& self, const std::byte& rhs)
    noexcept(has_copyability<P>(constraint_level::nothrow)) {
  std::construct_at(reinterpret_cast<P*>(&self),
      *std::launder(reinterpret_cast<const P*>(&rhs)));
}
template <std::size_t Len, std::size_t Align>
void copying_default_dispatcher(std::byte& self, const std::byte& rhs)
    noexcept {
  std::uninitialized_copy_n(
      std::assume_aligned<Align>(&rhs), Len, std::assume_aligned<Align>(&self));
}
template <class P>
void relocation_dispatcher(std::byte& self, const std::byte& rhs)
    noexcept(has_relocatability<P>(constraint_level::nothrow)) {
  P* other = std::launder(reinterpret_cast<P*>(const_cast<std::byte*>(&rhs)));
  std::construct_at(reinterpret_cast<P*>(&self), std::move(*other));
  std::destroy_at(other);
}
template <class P>
void destruction_dispatcher(std::byte& self)
    noexcept(has_destructibility<P>(constraint_level::nothrow))
    { std::destroy_at(std::launder(reinterpret_cast<P*>(&self))); }
inline void destruction_default_dispatcher(std::byte&) noexcept {}

template <bool IS_DIRECT, class O>
struct overload_traits : inapplicable_traits {};
template <bool NE, class R, class... Args>
struct indirect_overload_traits_impl : applicable_traits {
  using dispatcher_type = func_ptr_t<NE, R, const std::byte&, Args...>;
  template <class D>
  struct meta_provider {
    template <class P>
    static constexpr dispatcher_type get() {
      if constexpr (invocable_dispatch<
          D, NE, R, typename ptr_traits<P>::target_type&, Args...>) {
        return &indirect_conv_dispatcher<P, D, R, Args...>;
      } else {
        return &default_conv_dispatcher<
            D, qualifier_type::const_lv, R, Args...>;
      }
    }
  };
  struct resolver
      { indirect_overload_traits_impl operator()(const std::byte&, Args...); };
  template <class D, class P>
  static constexpr bool applicable_ptr =
      invocable_indirect_dispatch_ptr<D, P, NE, R, Args...>;
  static constexpr bool is_noexcept = NE;
};
template <qualifier_type Q, bool NE, class R, class... Args>
struct direct_overload_traits_impl : applicable_traits {
  using dispatcher_type = func_ptr_t<
      NE, R, add_qualifier_t<std::byte, Q>, Args...>;
  template <class D>
  struct meta_provider {
    template <class P>
    static constexpr dispatcher_type get() {
      if constexpr (invocable_dispatch<
          D, NE, R, add_qualifier_t<P, Q>, Args...>) {
        return &direct_conv_dispatcher<P, D, Q, R, Args...>;
      } else {
        return &default_conv_dispatcher<D, Q, R, Args...>;
      }
    }
  };
  struct resolver {
    direct_overload_traits_impl operator()(
        add_qualifier_t<std::byte, Q>, Args...);
  };
  template <class D, class P>
  static constexpr bool applicable_ptr =
      invocable_direct_dispatch_ptr<D, P, Q, NE, R, Args...>;
  static constexpr bool is_noexcept = NE;
};
template <class R, class... Args>
struct overload_traits<false, R(Args...)>
    : indirect_overload_traits_impl<false, R, Args...> {};
template <class R, class... Args>
struct overload_traits<false, R(Args...) noexcept>
    : indirect_overload_traits_impl<true, R, Args...> {};
template <class R, class... Args>
struct overload_traits<true, R(Args...)>
    : direct_overload_traits_impl<qualifier_type::lv, false, R, Args...> {};
template <class R, class... Args>
struct overload_traits<true, R(Args...) noexcept>
    : direct_overload_traits_impl<qualifier_type::lv, true, R, Args...> {};
template <class R, class... Args>
struct overload_traits<true, R(Args...) &>
    : direct_overload_traits_impl<qualifier_type::lv, false, R, Args...> {};
template <class R, class... Args>
struct overload_traits<true, R(Args...) & noexcept>
    : direct_overload_traits_impl<qualifier_type::lv, true, R, Args...> {};
template <class R, class... Args>
struct overload_traits<true, R(Args...) &&>
    : direct_overload_traits_impl<qualifier_type::rv, false, R, Args...> {};
template <class R, class... Args>
struct overload_traits<true, R(Args...) && noexcept>
    : direct_overload_traits_impl<qualifier_type::rv, true, R, Args...> {};
template <class R, class... Args>
struct overload_traits<true, R(Args...) const>
    : direct_overload_traits_impl<qualifier_type::const_lv, false, R, Args...>
    {};
template <class R, class... Args>
struct overload_traits<true, R(Args...) const noexcept>
    : direct_overload_traits_impl<qualifier_type::const_lv, true, R, Args...>
    {};
template <class R, class... Args>
struct overload_traits<true, R(Args...) const&>
    : direct_overload_traits_impl<qualifier_type::const_lv, false, R, Args...>
    {};
template <class R, class... Args>
struct overload_traits<true, R(Args...) const& noexcept>
    : direct_overload_traits_impl<qualifier_type::const_lv, true, R, Args...>
    {};
template <class R, class... Args>
struct overload_traits<true, R(Args...) const&&>
    : direct_overload_traits_impl<qualifier_type::const_rv, false, R, Args...>
    {};
template <class R, class... Args>
struct overload_traits<true, R(Args...) const&& noexcept>
    : direct_overload_traits_impl<qualifier_type::const_rv, true, R, Args...>
    {};

template <class T> struct nullable_traits : inapplicable_traits {};
template <class T>
    requires(
        requires(const T& cv, T& v) {
          { T{} } noexcept;
          { cv.has_value() } noexcept -> std::same_as<bool>;
          { v.reset() } noexcept;
        })
struct nullable_traits<T> : applicable_traits {};

template <class MP>
struct dispatcher_meta {
  constexpr dispatcher_meta() noexcept : dispatcher(nullptr) {}
  template <class P>
  constexpr explicit dispatcher_meta(std::in_place_type_t<P>) noexcept
      : dispatcher(MP::template get<P>()) {}
  bool has_value() const noexcept { return dispatcher != nullptr; }
  void reset() noexcept { dispatcher = nullptr; }

  decltype(MP::template get<void>()) dispatcher;
};

template <class... Ms>
struct composite_meta_impl : Ms... {
  static constexpr bool is_nullable =
      requires { typename first_applicable_t<nullable_traits, Ms...>; };

  constexpr composite_meta_impl() noexcept requires(is_nullable) = default;
  template <class P>
  constexpr explicit composite_meta_impl(std::in_place_type_t<P>) noexcept
      : Ms(std::in_place_type<P>)... {}

  bool has_value() const noexcept requires(is_nullable)
      { return first_applicable_t<nullable_traits, Ms...>::has_value(); }
  void reset() noexcept requires(is_nullable)
      { first_applicable_t<nullable_traits, Ms...>::reset(); }
};
template <class O, class I> struct meta_reduction : std::type_identity<O> {};
template <class... Ms, class I> requires(!std::is_void_v<I>)
struct meta_reduction<composite_meta_impl<Ms...>, I>
    : std::type_identity<composite_meta_impl<Ms..., I>> {};
template <class... Ms1, class... Ms2>
struct meta_reduction<composite_meta_impl<Ms1...>, composite_meta_impl<Ms2...>>
    : std::type_identity<composite_meta_impl<Ms1..., Ms2...>> {};
template <class O, class I>
using meta_reduction_t = typename meta_reduction<O, I>::type;
template <class... Ms>
using composite_meta =
    recursive_reduction_t<meta_reduction_t, composite_meta_impl<>, Ms...>;

template <class T, class F>
using proxy_accessor = typename T::template accessor<proxy<F>>;
template <class T>
consteval bool is_meta_is_direct_well_formed() {
  if constexpr (requires { { T::is_direct } -> std::same_as<const bool&>; }) {
    if constexpr (is_consteval([] { return T::is_direct; })) {
      return true;
    }
  }
  return false;
}
template <class T, class F>
consteval bool is_a11y_well_formed() {
  if constexpr (requires { typename proxy_accessor<T, F>; }) {
    return !std::is_final_v<proxy_accessor<T, F>> &&
        std::is_nothrow_default_constructible_v<proxy_accessor<T, F>>;
  } else {
    return true;
  }
  return false;
}

template <class C, class... Os>
struct conv_traits_impl : inapplicable_traits {};
template <class C, class... Os>
    requires(sizeof...(Os) > 0u && (overload_traits<
        C::dispatch_type::is_direct, Os>::applicable && ...))
struct conv_traits_impl<C, Os...> : applicable_traits {
 private:
  struct overload_resolver : overload_traits<
      C::dispatch_type::is_direct, Os>::resolver... {
    using overload_traits<C::dispatch_type::is_direct, Os>::resolver
        ::operator()...;
  };

 public:
  using dispatch_type = typename C::dispatch_type;
  using meta = composite_meta_impl<dispatcher_meta<
      typename overload_traits<C::dispatch_type::is_direct, Os>
          ::template meta_provider<dispatch_type>>...>;
  template <class... Args>
  using matched_overload_traits =
      std::invoke_result_t<overload_resolver, Args...>;

  template <class P>
  static constexpr bool applicable_ptr = (overload_traits<
      C::dispatch_type::is_direct, Os>
          ::template applicable_ptr<dispatch_type, P> && ...);
};
template <class C> struct conv_traits : inapplicable_traits {};
template <class C>
    requires(
        requires {
          typename C::dispatch_type;
          typename C::overload_types;
        } &&
        std::is_nothrow_default_constructible_v<typename C::dispatch_type> &&
        is_meta_is_direct_well_formed<typename C::dispatch_type>() &&
        is_tuple_like_well_formed<typename C::overload_types>())
struct conv_traits<C>
    : instantiated_t<conv_traits_impl, typename C::overload_types, C> {};

template <class R>
struct indirect_refl_meta : R {
  constexpr indirect_refl_meta()
      noexcept(std::is_nothrow_default_constructible_v<R>)
      requires(std::is_default_constructible_v<R>) = default;
  template <class P>
  constexpr explicit indirect_refl_meta(std::in_place_type_t<P>)
      noexcept(std::is_nothrow_constructible_v<
          R, std::in_place_type_t<typename ptr_traits<P>::target_type>>)
      requires(std::is_constructible_v<
          R, std::in_place_type_t<typename ptr_traits<P>::target_type>>)
      : R(std::in_place_type<typename ptr_traits<P>::target_type>) {}
};

template <bool NE>
struct copyability_meta_provider {
  template <class P>
  static constexpr func_ptr_t<NE, void, std::byte&, const std::byte&> get() {
    if constexpr (has_copyability<P>(constraint_level::trivial)) {
      return &copying_default_dispatcher<sizeof(P), alignof(P)>;
    } else {
      return &copying_dispatcher<P>;
    }
  }
};
template <bool NE>
struct relocatability_meta_provider {
  template <class P>
  static constexpr func_ptr_t<NE, void, std::byte&, const std::byte&> get() {
    if constexpr (has_relocatability<P>(constraint_level::trivial)) {
      return &copying_default_dispatcher<sizeof(P), alignof(P)>;
    } else {
      return &relocation_dispatcher<P>;
    }
  }
};
template <bool NE>
struct destructibility_meta_provider {
  template <class P>
  static constexpr func_ptr_t<NE, void, std::byte&> get() {
    if constexpr (has_destructibility<P>(constraint_level::trivial)) {
      return &destruction_default_dispatcher;
    } else {
      return &destruction_dispatcher<P>;
    }
  }
};
template <template <bool> class MP, constraint_level C>
struct lifetime_meta_traits : std::type_identity<void> {};
template <template <bool> class MP>
struct lifetime_meta_traits<MP, constraint_level::nothrow>
    : std::type_identity<dispatcher_meta<MP<true>>> {};
template <template <bool> class MP>
struct lifetime_meta_traits<MP, constraint_level::nontrivial>
    : std::type_identity<dispatcher_meta<MP<false>>> {};
template <template <bool> class MP, constraint_level C>
using lifetime_meta_t = typename lifetime_meta_traits<MP, C>::type;

template <class... As>
class composite_accessor_impl : public As... {
  template <class> friend class pro::proxy;

  composite_accessor_impl() noexcept = default;
  composite_accessor_impl(const composite_accessor_impl&) noexcept = default;
  composite_accessor_impl& operator=(const composite_accessor_impl&) noexcept
      = default;
};
template <class F, bool IS_DIRECT, class O, class I>
struct composite_accessor_reduction : std::type_identity<O> {};
template <class F, bool IS_DIRECT, class... As, class I>
    requires((I::is_direct == IS_DIRECT) &&
        requires { typename proxy_accessor<I, F>; })
struct composite_accessor_reduction<
    F, IS_DIRECT, composite_accessor_impl<As...>, I>
    : std::type_identity<composite_accessor_impl<
          As..., proxy_accessor<I, F>>> {};
template <class F, bool IS_DIRECT>
struct composite_accessor_helper {
  template <class O, class I>
  using reduction_t =
      typename composite_accessor_reduction<F, IS_DIRECT, O, I>::type;
};
template <class F, bool IS_DIRECT, class... Ts>
using composite_accessor = recursive_reduction_t<composite_accessor_helper<
    F, IS_DIRECT>::template reduction_t, composite_accessor_impl<>, Ts...>;

template <class As1, class As2> struct merge_composite_accessor_traits;
template <class... As1, class... As2>
struct merge_composite_accessor_traits<
    composite_accessor_impl<As1...>, composite_accessor_impl<As2...>>
    : std::type_identity<composite_accessor_impl<As1..., As2...>> {};
template <class T, class U>
using merged_composite_accessor =
    typename merge_composite_accessor_traits<T, U>::type;

template <class F>
consteval bool is_facade_constraints_well_formed() {
  if constexpr (requires {
      { F::constraints } -> std::same_as<const proxiable_ptr_constraints&>; }) {
    if constexpr (is_consteval([] { return F::constraints; })) {
      return std::has_single_bit(F::constraints.max_align) &&
          F::constraints.max_size % F::constraints.max_align == 0u;
    }
  }
  return false;
}
template <class R, class P>
consteval bool is_reflection_type_well_formed() {
  using T = std::conditional_t<R::is_direct, P,
      typename ptr_traits<P>::target_type>;
  if constexpr (std::is_constructible_v<R, std::in_place_type_t<T>>) {
    if constexpr (is_consteval([] { return R{std::in_place_type<T>}; })) {
      return true;
    }
  }
  return false;
}
template <class D>
struct dispatch_match_helper {
  template <class C>
  using traits =
      conditional_traits<std::is_same_v<typename C::dispatch_type, D>>;
};
template <class D, class C>
struct dispatch_match_traits : inapplicable_traits {};
template <class D, class C>
    requires(std::is_same_v<typename C::dispatch_type, D>)
struct dispatch_match_traits<D, C> : applicable_traits {};
template <class F, class... Cs>
struct facade_conv_traits_impl : inapplicable_traits {};
template <class F, class... Cs> requires((conv_traits<Cs>::applicable && ...) &&
    (is_a11y_well_formed<typename Cs::dispatch_type, F>() && ...))
struct facade_conv_traits_impl<F, Cs...> : applicable_traits {
  using conv_meta = composite_meta<typename conv_traits<Cs>::meta...>;
  using direct_conv_accessor =
      composite_accessor<F, true, typename conv_traits<Cs>::dispatch_type...>;
  using indirect_conv_accessor =
      composite_accessor<F, false, typename conv_traits<Cs>::dispatch_type...>;
  template <class D, class... Args>
  using matched_overload_traits = typename conv_traits<first_applicable_t<
      dispatch_match_helper<D>::template traits, Cs...>>
      ::template matched_overload_traits<Args...>;

  template <class P>
  static constexpr bool conv_applicable_ptr =
      (conv_traits<Cs>::template applicable_ptr<P> && ...);
  static constexpr bool conv_has_indirection =
      (!Cs::dispatch_type::is_direct || ...);
};
template <class F, class... Rs>
struct facade_refl_traits_impl : inapplicable_traits {};
template <class F, class... Rs>
    requires((is_meta_is_direct_well_formed<Rs>() && ...) &&
        (is_a11y_well_formed<Rs, F>() && ...))
struct facade_refl_traits_impl<F, Rs...> : applicable_traits {
  using refl_meta = composite_meta<std::conditional_t<
      Rs::is_direct, Rs, indirect_refl_meta<Rs>>...>;
  using direct_refl_accessor = composite_accessor<F, true, Rs...>;
  using indirect_refl_accessor = composite_accessor<F, false, Rs...>;

  template <class R>
  static constexpr bool has_refl = (std::is_same_v<R, Rs> || ...);
  template <class P>
  static constexpr bool refl_applicable_ptr =
      (is_reflection_type_well_formed<Rs, P>() && ...);
  static constexpr bool refl_has_indirection = (!Rs::is_direct || ...);
};
template <class F> struct facade_traits : inapplicable_traits {};
template <class F>
    requires(
        requires {
          typename F::convention_types;
          typename F::reflection_types;
        } &&
        is_facade_constraints_well_formed<F>() &&
        is_tuple_like_well_formed<typename F::convention_types>() &&
        instantiated_t<facade_conv_traits_impl, typename F::convention_types, F>
            ::applicable &&
        is_tuple_like_well_formed<typename F::reflection_types>() &&
        instantiated_t<facade_refl_traits_impl, typename F::reflection_types, F>
            ::applicable)
struct facade_traits<F>
    : instantiated_t<facade_conv_traits_impl, typename F::convention_types, F>,
      instantiated_t<facade_refl_traits_impl, typename F::reflection_types, F> {
  using copyability_meta = lifetime_meta_t<
      copyability_meta_provider, F::constraints.copyability>;
  using relocatability_meta = lifetime_meta_t<
      relocatability_meta_provider, F::constraints.relocatability>;
  using destructibility_meta = lifetime_meta_t<
      destructibility_meta_provider, F::constraints.destructibility>;
  using meta = composite_meta<copyability_meta, relocatability_meta,
      destructibility_meta, typename facade_traits::conv_meta,
      typename facade_traits::refl_meta>;
  using direct_accessor = merged_composite_accessor<
      typename facade_traits::direct_conv_accessor,
      typename facade_traits::direct_refl_accessor>;
  using indirect_accessor = merged_composite_accessor<
      typename facade_traits::indirect_conv_accessor,
      typename facade_traits::indirect_refl_accessor>;

  static constexpr bool applicable = true;
  static constexpr bool has_indirection = facade_traits::conv_has_indirection ||
      facade_traits::refl_has_indirection;
};

using ptr_prototype = void*[2];

template <class M>
struct meta_ptr {
  constexpr meta_ptr() noexcept : ptr_(nullptr) {};
  template <class P>
  constexpr explicit meta_ptr(std::in_place_type_t<P>) noexcept
      : ptr_(&storage<P>) {}
  bool has_value() const noexcept { return ptr_ != nullptr; }
  void reset() noexcept { ptr_ = nullptr; }
  const M* operator->() const noexcept { return ptr_; }

 private:
  const M* ptr_;
  template <class P> static constexpr M storage{std::in_place_type<P>};
};
template <class M>
    requires(sizeof(M) <= sizeof(ptr_prototype) &&
        alignof(M) <= alignof(ptr_prototype) && nullable_traits<M>::applicable)
struct meta_ptr<M> : M {
  using M::M;
  const M* operator->() const noexcept { return this; }
};

template <class F>
struct proxy_helper {
  template <class D, class P, class... Args>
  using matched_overload_traits = typename facade_traits<F>
      ::template matched_overload_traits<D, add_qualifier_t<
          std::byte, qualifier_of_v<P&&>>, Args...>;

  static inline const auto& get_meta(const proxy<F>& p) noexcept
      { return *p.meta_.operator->(); }
  template <class D, class P, class... Args>
  static decltype(auto) invoke(P&& p, Args&&... args)
      noexcept(matched_overload_traits<D, P, Args...>::is_noexcept)
      requires(requires { typename matched_overload_traits<D, P, Args...>; }) {
    return p.meta_->template dispatcher_meta<typename matched_overload_traits<
        D, P, Args...>::template meta_provider<D>>::dispatcher(
        std::forward<add_qualifier_t<std::byte, qualifier_of_v<P&&>>>(*p.ptr_),
        std::forward<Args>(args)...);
  }
  static inline const proxy<F>& access(
      const typename facade_traits<F>::indirect_accessor& ia) {
    return *reinterpret_cast<const proxy<F>*>(
        reinterpret_cast<const std::byte*>(&ia) - offsetof(proxy<F>, ia_));
  }
};

}  // namespace details

template <class F>
concept facade = details::facade_traits<F>::applicable;

template <class P, class F>
concept proxiable = facade<F> && sizeof(P) <= F::constraints.max_size &&
    alignof(P) <= F::constraints.max_align &&
    details::has_copyability<P>(F::constraints.copyability) &&
    details::has_relocatability<P>(F::constraints.relocatability) &&
    details::has_destructibility<P>(F::constraints.destructibility) &&
    (!details::facade_traits<F>::has_indirection ||
        details::ptr_traits<P>::applicable) &&
    details::facade_traits<F>::template conv_applicable_ptr<P> &&
    details::facade_traits<F>::template refl_applicable_ptr<P>;

template <class F>
class proxy : public details::facade_traits<F>::direct_accessor {
  friend struct details::proxy_helper<F>;
  using Traits = details::facade_traits<F>;
  static_assert(Traits::applicable);
  using Meta = typename Traits::meta;

  template <class P, class... Args>
  static constexpr bool HasNothrowPolyConstructor = std::conditional_t<
      proxiable<P, F>, std::is_nothrow_constructible<P, Args...>,
          std::false_type>::value;
  template <class P, class... Args>
  static constexpr bool HasPolyConstructor = std::conditional_t<
      proxiable<P, F>, std::is_constructible<P, Args...>,
          std::false_type>::value;
  static constexpr bool HasTrivialCopyConstructor =
      F::constraints.copyability == constraint_level::trivial;
  static constexpr bool HasNothrowCopyConstructor =
      F::constraints.copyability >= constraint_level::nothrow;
  static constexpr bool HasCopyConstructor =
      F::constraints.copyability >= constraint_level::nontrivial;
  static constexpr bool HasNothrowMoveConstructor =
      F::constraints.relocatability >= constraint_level::nothrow;
  static constexpr bool HasMoveConstructor =
      F::constraints.relocatability >= constraint_level::nontrivial;
  static constexpr bool HasTrivialDestructor =
      F::constraints.destructibility == constraint_level::trivial;
  static constexpr bool HasNothrowDestructor =
      F::constraints.destructibility >= constraint_level::nothrow;
  static constexpr bool HasDestructor =
      F::constraints.destructibility >= constraint_level::nontrivial;
  template <class P, class... Args>
  static constexpr bool HasNothrowPolyAssignment =
      HasNothrowPolyConstructor<P, Args...> && HasNothrowDestructor;
  template <class P, class... Args>
  static constexpr bool HasPolyAssignment = HasPolyConstructor<P, Args...> &&
      HasDestructor;
  static constexpr bool HasTrivialCopyAssignment = HasTrivialCopyConstructor &&
      HasTrivialDestructor;
  static constexpr bool HasNothrowCopyAssignment = HasNothrowCopyConstructor &&
      HasNothrowDestructor;
  static constexpr bool HasCopyAssignment = HasNothrowCopyAssignment ||
      (HasCopyConstructor && HasMoveConstructor && HasDestructor);
  static constexpr bool HasNothrowMoveAssignment = HasNothrowMoveConstructor &&
      HasNothrowDestructor;
  static constexpr bool HasMoveAssignment = HasMoveConstructor && HasDestructor;

 public:
  using facade_type = F;

  proxy() noexcept = default;
  proxy(std::nullptr_t) noexcept : proxy() {}
  proxy(const proxy& rhs) noexcept(HasNothrowCopyConstructor)
      requires(!HasTrivialCopyConstructor && HasCopyConstructor) {
    if (rhs.meta_.has_value()) {
      rhs.meta_->Traits::copyability_meta::dispatcher(*ptr_, *rhs.ptr_);
      meta_ = rhs.meta_;
    } else {
      meta_.reset();
    }
  }
  proxy(const proxy&) noexcept requires(HasTrivialCopyConstructor) = default;
  proxy(const proxy&) requires(!HasCopyConstructor) = delete;
  proxy(proxy&& rhs) noexcept(HasNothrowMoveConstructor)
      requires(HasMoveConstructor) {
    if (rhs.meta_.has_value()) {
      if constexpr (F::constraints.relocatability ==
          constraint_level::trivial) {
        std::ranges::uninitialized_copy(rhs.ptr_, ptr_);
      } else {
        rhs.meta_->Traits::relocatability_meta::dispatcher(*ptr_, *rhs.ptr_);
      }
      meta_ = rhs.meta_;
      rhs.meta_.reset();
    } else {
      meta_.reset();
    }
  }
  proxy(proxy&&) requires(!HasMoveConstructor) = delete;
  template <class P>
  proxy(P&& ptr) noexcept(HasNothrowPolyConstructor<std::decay_t<P>, P>)
      requires(HasPolyConstructor<std::decay_t<P>, P>)
      { initialize<std::decay_t<P>>(std::forward<P>(ptr)); }
  template <class P, class... Args>
  explicit proxy(std::in_place_type_t<P>, Args&&... args)
      noexcept(HasNothrowPolyConstructor<P, Args...>)
      requires(HasPolyConstructor<P, Args...>)
      { initialize<P>(std::forward<Args>(args)...); }
  template <class P, class U, class... Args>
  explicit proxy(std::in_place_type_t<P>, std::initializer_list<U> il,
          Args&&... args)
      noexcept(HasNothrowPolyConstructor<P, std::initializer_list<U>&, Args...>)
      requires(HasPolyConstructor<P, std::initializer_list<U>&, Args...>)
      { initialize<P>(il, std::forward<Args>(args)...); }
  proxy& operator=(std::nullptr_t) noexcept(HasNothrowDestructor)
      requires(HasDestructor) {
    std::destroy_at(this);
    std::construct_at(this);
    return *this;
  }
  proxy& operator=(const proxy& rhs)
      requires(!HasNothrowCopyAssignment && HasCopyAssignment)
      { return *this = proxy{rhs}; }
  proxy& operator=(const proxy& rhs) noexcept
      requires(!HasTrivialCopyAssignment && HasNothrowCopyAssignment) {
    if (this != &rhs) {
      std::destroy_at(this);
      std::construct_at(this, rhs);
    }
    return *this;
  }
  proxy& operator=(const proxy&) noexcept requires(HasTrivialCopyAssignment) =
      default;
  proxy& operator=(const proxy&) requires(!HasCopyAssignment) = delete;
  proxy& operator=(proxy&& rhs) noexcept(HasNothrowMoveAssignment)
    requires(HasMoveAssignment) {
    if (this != &rhs) {
      if constexpr (HasNothrowMoveAssignment) {
        std::destroy_at(this);
      } else {
        reset();  // For weak exception safety
      }
      std::construct_at(this, std::move(rhs));
    }
    return *this;
  }
  proxy& operator=(proxy&&) requires(!HasMoveAssignment) = delete;
  template <class P>
  proxy& operator=(P&& ptr) noexcept
      requires(HasNothrowPolyAssignment<std::decay_t<P>, P>) {
    std::destroy_at(this);
    initialize<std::decay_t<P>>(std::forward<P>(ptr));
    return *this;
  }
  template <class P>
  proxy& operator=(P&& ptr)
      requires(!HasNothrowPolyAssignment<std::decay_t<P>, P> &&
          HasPolyAssignment<std::decay_t<P>, P>)
      { return *this = proxy{std::forward<P>(ptr)}; }
  ~proxy() noexcept(HasNothrowDestructor)
      requires(!HasTrivialDestructor && HasDestructor) {
    if (meta_.has_value()) {
      meta_->Traits::destructibility_meta::dispatcher(*ptr_);
    }
  }
  ~proxy() requires(HasTrivialDestructor) = default;
  ~proxy() requires(!HasDestructor) = delete;

  bool has_value() const noexcept { return meta_.has_value(); }
  void reset() noexcept(HasNothrowDestructor) requires(HasDestructor)
      { std::destroy_at(this); meta_.reset(); }
  void swap(proxy& rhs) noexcept(HasNothrowMoveConstructor)
      requires(HasMoveConstructor) {
    if constexpr (F::constraints.relocatability == constraint_level::trivial) {
      std::swap(meta_, rhs.meta_);
      std::swap(ptr_, rhs.ptr);
    } else {
      if (meta_.has_value()) {
        if (rhs.meta_.has_value()) {
          proxy temp = std::move(*this);
          std::construct_at(this, std::move(rhs));
          std::construct_at(&rhs, std::move(temp));
        } else {
          std::construct_at(&rhs, std::move(*this));
        }
      } else if (rhs.meta_.has_value()) {
        std::construct_at(this, std::move(rhs));
      }
    }
  }
  friend void swap(proxy& a, proxy& b) noexcept(HasNothrowMoveConstructor)
      { a.swap(b); }
  template <class P, class... Args>
  P& emplace(Args&&... args) noexcept(HasNothrowPolyAssignment<P, Args...>)
      requires(HasPolyAssignment<P, Args...>) {
    reset();
    return initialize<P>(std::forward<Args>(args)...);
  }
  template <class P, class U, class... Args>
  P& emplace(std::initializer_list<U> il, Args&&... args)
      noexcept(HasNothrowPolyAssignment<P, std::initializer_list<U>&, Args...>)
      requires(HasPolyAssignment<P, std::initializer_list<U>&, Args...>) {
    reset();
    return initialize<P>(il, std::forward<Args>(args)...);
  }
  auto operator->() const noexcept requires(Traits::has_indirection)
      { return &ia_; }
  auto& operator*() const noexcept requires(Traits::has_indirection)
      { return ia_; }

 private:
  template <class P, class... Args>
  P& initialize(Args&&... args) {
    std::construct_at(reinterpret_cast<P*>(ptr_), std::forward<Args>(args)...);
    meta_ = details::meta_ptr<Meta>{std::in_place_type<P>};
    return *std::launder(reinterpret_cast<P*>(ptr_));
  }

  [[___PRO_NO_UNIQUE_ADDRESS_ATTRIBUTE]]
  mutable typename Traits::indirect_accessor ia_;
  details::meta_ptr<Meta> meta_;
  alignas(F::constraints.max_align) std::byte ptr_[F::constraints.max_size];
};

template <class D, facade F, class... Args>
decltype(auto) proxy_invoke(proxy<F>& p, Args&&... args)
    ___PRO_DIRECT_FUNC_IMPL(details::proxy_helper<F>::template invoke<D>(
        p, std::forward<Args>(args)...))
template <class D, facade F, class... Args>
decltype(auto) proxy_invoke(const proxy<F>& p, Args&&... args)
    ___PRO_DIRECT_FUNC_IMPL(details::proxy_helper<F>::template invoke<D>(
        p, std::forward<Args>(args)...))
template <class D, facade F, class... Args>
decltype(auto) proxy_invoke(proxy<F>&& p, Args&&... args)
    ___PRO_DIRECT_FUNC_IMPL(details::proxy_helper<F>::template invoke<D>(
        static_cast<proxy<F>&&>(p), std::forward<Args>(args)...))
template <class D, facade F, class... Args>
decltype(auto) proxy_invoke(const proxy<F>&& p, Args&&... args)
    ___PRO_DIRECT_FUNC_IMPL(details::proxy_helper<F>::template invoke<D>(
        static_cast<const proxy<F>&&>(p), std::forward<Args>(args)...))

template <class R, facade F>
const R& proxy_reflect(const proxy<F>& p) noexcept
    requires(details::facade_traits<F>::template has_refl<R>)
    { return details::proxy_helper<F>::get_meta(p); }

template <class P, class A>
decltype(auto) access_proxy(A&& a) noexcept {
  if constexpr (std::is_base_of_v<std::remove_cvref_t<A>, P>) {
    return static_cast<details::add_qualifier_t<
        P, details::qualifier_of_v<A&&>>>(std::forward<A>(a));
  } else {
    using IA = typename details::facade_traits<typename P::facade_type>
        ::indirect_accessor;
    static_assert(std::is_base_of_v<std::remove_cvref_t<A>, IA>);
    return details::proxy_helper<typename P::facade_type>
        ::access(static_cast<const IA&>(a));
  }
}

namespace details {

template <class T>
class inplace_ptr {
 public:
  template <class... Args>
  inplace_ptr(Args&&... args)
      noexcept(std::is_nothrow_constructible_v<T, Args...>)
      requires(std::is_constructible_v<T, Args...>)
      : value_(std::forward<Args>(args)...) {}
  inplace_ptr(const inplace_ptr&)
      noexcept(std::is_nothrow_copy_constructible_v<T>) = default;
  inplace_ptr(inplace_ptr&&)
      noexcept(std::is_nothrow_move_constructible_v<T>) = default;

  T* operator->() const noexcept { return &value_; }

 private:
  mutable T value_;
};

#if __STDC_HOSTED__
template <class T, class Alloc>
static auto rebind_allocator(const Alloc& alloc) {
  return typename std::allocator_traits<Alloc>::template rebind_alloc<T>(alloc);
}
template <class T, class Alloc, class... Args>
static T* allocate(const Alloc& alloc, Args&&... args) {
  auto al = rebind_allocator<T>(alloc);
  auto deleter = [&](T* ptr) { al.deallocate(ptr, 1); };
  std::unique_ptr<T, decltype(deleter)> result{al.allocate(1), deleter};
  std::construct_at(result.get(), std::forward<Args>(args)...);
  return result.release();
}
template <class Alloc, class T>
static void deallocate(const Alloc& alloc, T* ptr) {
  auto al = rebind_allocator<T>(alloc);
  std::destroy_at(ptr);
  al.deallocate(ptr, 1);
}

template <class T, class Alloc>
class allocated_ptr {
 public:
  template <class... Args>
  allocated_ptr(const Alloc& alloc, Args&&... args)
      requires(std::is_constructible_v<T, Args...>)
      : alloc_(alloc), ptr_(allocate<T>(alloc, std::forward<Args>(args)...)) {}
  allocated_ptr(const allocated_ptr& rhs)
      requires(std::is_copy_constructible_v<T>)
      : alloc_(rhs.alloc_), ptr_(rhs.ptr_ == nullptr ? nullptr :
            allocate<T>(alloc_, std::as_const(*rhs.ptr_))) {}
  allocated_ptr(allocated_ptr&& rhs)
      noexcept(std::is_nothrow_move_constructible_v<Alloc>)
      : alloc_(std::move(rhs.alloc_)), ptr_(std::exchange(rhs.ptr_, nullptr)) {}
  ~allocated_ptr() { if (ptr_ != nullptr) { deallocate(alloc_, ptr_); } }

  T* operator->() const noexcept { return ptr_; }

 private:
  [[___PRO_NO_UNIQUE_ADDRESS_ATTRIBUTE]]
  Alloc alloc_;
  T* ptr_;
};
template <class T, class Alloc>
class compact_ptr {
 public:
  template <class... Args>
  compact_ptr(const Alloc& alloc, Args&&... args)
      requires(std::is_constructible_v<T, Args...>)
      : ptr_(allocate<storage>(alloc, alloc, std::forward<Args>(args)...)) {}
  compact_ptr(const compact_ptr& rhs) requires(std::is_copy_constructible_v<T>)
      : ptr_(rhs.ptr_ == nullptr ? nullptr : allocate<storage>(rhs.ptr_->alloc,
            rhs.ptr_->alloc, std::as_const(rhs.ptr_->value))) {}
  compact_ptr(compact_ptr&& rhs) noexcept
      : ptr_(std::exchange(rhs.ptr_, nullptr)) {}
  ~compact_ptr() { if (ptr_ != nullptr) { deallocate(ptr_->alloc, ptr_); } }

  T* operator->() const noexcept { return &ptr_->value; }

 private:
  struct storage {
    template <class... Args>
    explicit storage(const Alloc& alloc, Args&&... args)
        : value(std::forward<Args>(args)...), alloc(alloc) {}

    T value;
    Alloc alloc;
  };

  storage* ptr_;
};
template <class F, class T, class Alloc, class... Args>
proxy<F> allocate_proxy_impl(const Alloc& alloc, Args&&... args) {
  if constexpr (proxiable<allocated_ptr<T, Alloc>, F>) {
    return proxy<F>{std::in_place_type<allocated_ptr<T, Alloc>>,
        alloc, std::forward<Args>(args)...};
  } else {
    return proxy<F>{std::in_place_type<compact_ptr<T, Alloc>>,
        alloc, std::forward<Args>(args)...};
  }
}
template <class F, class T, class... Args>
proxy<F> make_proxy_impl(Args&&... args) {
  if constexpr (proxiable<inplace_ptr<T>, F>) {
    return proxy<F>{std::in_place_type<inplace_ptr<T>>,
        std::forward<Args>(args)...};
  } else {
    return allocate_proxy_impl<F, T>(
        std::allocator<T>{}, std::forward<Args>(args)...);
  }
}
#endif  // __STDC_HOSTED__

}  // namespace details

template <class T, class F>
concept inplace_proxiable_target = proxiable<details::inplace_ptr<T>, F>;

template <facade F, inplace_proxiable_target<F> T, class... Args>
proxy<F> make_proxy_inplace(Args&&... args)
    noexcept(std::is_nothrow_constructible_v<T, Args...>) {
  return proxy<F>{std::in_place_type<details::inplace_ptr<T>>,
      std::forward<Args>(args)...};
}
template <facade F, inplace_proxiable_target<F> T, class U, class... Args>
proxy<F> make_proxy_inplace(std::initializer_list<U> il, Args&&... args)
    noexcept(std::is_nothrow_constructible_v<
        T, std::initializer_list<U>&, Args...>) {
  return proxy<F>{std::in_place_type<details::inplace_ptr<T>>,
      il, std::forward<Args>(args)...};
}
template <facade F, class T>
proxy<F> make_proxy_inplace(T&& value)
    noexcept(std::is_nothrow_constructible_v<std::decay_t<T>, T>)
    requires(inplace_proxiable_target<std::decay_t<T>, F>) {
  return proxy<F>{std::in_place_type<details::inplace_ptr<std::decay_t<T>>>,
      std::forward<T>(value)};
}

#if __STDC_HOSTED__
template <facade F, class T, class Alloc, class... Args>
proxy<F> allocate_proxy(const Alloc& alloc, Args&&... args) {
  return details::allocate_proxy_impl<F, T>(alloc, std::forward<Args>(args)...);
}
template <facade F, class T, class Alloc, class U,
    class... Args>
proxy<F> allocate_proxy(const Alloc& alloc, std::initializer_list<U> il,
    Args&&... args) {
  return details::allocate_proxy_impl<F, T>(
      alloc, il, std::forward<Args>(args)...);
}
template <facade F, class Alloc, class T>
proxy<F> allocate_proxy(const Alloc& alloc, T&& value) {
  return details::allocate_proxy_impl<F, std::decay_t<T>>(
      alloc, std::forward<T>(value));
}
template <facade F, class T, class... Args>
proxy<F> make_proxy(Args&&... args)
    { return details::make_proxy_impl<F, T>(std::forward<Args>(args)...); }
template <facade F, class T, class U, class... Args>
proxy<F> make_proxy(std::initializer_list<U> il, Args&&... args)
    { return details::make_proxy_impl<F, T>(il, std::forward<Args>(args)...); }
template <facade F, class T>
proxy<F> make_proxy(T&& value) {
  return details::make_proxy_impl<F, std::decay_t<T>>(std::forward<T>(value));
}
#endif  // __STDC_HOSTED__

// The following types and macros aim to simplify definition of dispatch,
// convention, and facade types prior to C++26
namespace details {

constexpr std::size_t invalid_size = static_cast<std::size_t>(-1);
constexpr constraint_level invalid_cl = static_cast<constraint_level>(-1);
consteval auto normalize(proxiable_ptr_constraints value) {
  if (value.max_size == invalid_size)
      { value.max_size = sizeof(ptr_prototype); }
  if (value.max_align == invalid_size)
      { value.max_align = alignof(ptr_prototype); }
  if (value.copyability == invalid_cl)
      { value.copyability = constraint_level::none; }
  if (value.relocatability == invalid_cl)
      { value.relocatability = constraint_level::nothrow; }
  if (value.destructibility == invalid_cl)
      { value.destructibility = constraint_level::nothrow; }
  return value;
}
consteval auto make_restricted_layout(proxiable_ptr_constraints value,
    std::size_t max_size, std::size_t max_align) {
  if (value.max_size == invalid_size || value.max_size > max_size)
      { value.max_size = max_size; }
  if (value.max_align == invalid_size || value.max_align > max_align)
      { value.max_align = max_align; }
  return value;
}
consteval auto make_copyable(proxiable_ptr_constraints value,
    constraint_level cl) {
  if (value.copyability == invalid_cl || value.copyability < cl)
      { value.copyability = cl; }
  return value;
}
consteval auto make_relocatable(proxiable_ptr_constraints value,
    constraint_level cl) {
  if (value.relocatability == invalid_cl || value.relocatability < cl)
      { value.relocatability = cl; }
  return value;
}
consteval auto make_destructible(proxiable_ptr_constraints value,
    constraint_level cl) {
  if (value.destructibility == invalid_cl || value.destructibility < cl)
      { value.destructibility = cl; }
  return value;
}
consteval auto merge_constraints(proxiable_ptr_constraints a,
    proxiable_ptr_constraints b) {
  a = make_restricted_layout(a, b.max_size, b.max_align);
  a = make_copyable(a, b.copyability);
  a = make_relocatable(a, b.relocatability);
  a = make_destructible(a, b.destructibility);
  return a;
}

template <class D, class Os>
struct conv_impl {
  using dispatch_type = D;
  using overload_types = Os;
};
template <class Cs, class Rs, proxiable_ptr_constraints C>
struct facade_impl {
  using convention_types = Cs;
  using reflection_types = Rs;
  static constexpr proxiable_ptr_constraints constraints = C;
};

template <class O, class I>
struct add_tuple_reduction : std::type_identity<O> {};
template <class... Os, class I> requires(!std::is_same_v<I, Os> && ...)
struct add_tuple_reduction<std::tuple<Os...>, I>
    : std::type_identity<std::tuple<Os..., I>> {};
template <class T, class U>
using add_tuple_t = typename add_tuple_reduction<T, U>::type;
template <class O, class... Is>
using merge_tuple_impl_t = recursive_reduction_t<add_tuple_t, O, Is...>;
template <class T, class U>
using merge_tuple_t = instantiated_t<merge_tuple_impl_t, U, T>;

template <class C0, class C1>
using merge_conv_t = conv_impl<typename C0::dispatch_type, merge_tuple_t<
    typename C0::overload_types, typename C1::overload_types>>;

template <class Cs0, class C1, class C> struct add_conv_reduction;
template <class... Cs0, class C1, class... Cs2, class C>
struct add_conv_reduction<std::tuple<Cs0...>, std::tuple<C1, Cs2...>, C>
    : add_conv_reduction<std::tuple<Cs0..., C1>, std::tuple<Cs2...>, C> {};
template <class... Cs0, class C1, class... Cs2, class C>
    requires(std::is_same_v<
        typename C::dispatch_type, typename C1::dispatch_type>)
struct add_conv_reduction<std::tuple<Cs0...>, std::tuple<C1, Cs2...>, C>
    : std::type_identity<std::tuple<Cs0..., merge_conv_t<C1, C>, Cs2...>> {};
template <class... Cs, class C>
struct add_conv_reduction<std::tuple<Cs...>, std::tuple<>, C>
    : std::type_identity<std::tuple<Cs..., merge_conv_t<
          conv_impl<typename C::dispatch_type, std::tuple<>>, C>>> {};
template <class Cs, class C>
using add_conv_t = typename add_conv_reduction<std::tuple<>, Cs, C>::type;

template <class Cs0, class... Cs1>
using merge_conv_tuple_impl_t = recursive_reduction_t<add_conv_t, Cs0, Cs1...>;
template <class Cs0, class Cs1>
using merge_conv_tuple_t = instantiated_t<merge_conv_tuple_impl_t, Cs1, Cs0>;

template <class Cs, class Rs, proxiable_ptr_constraints C>
struct facade_builder_impl {
  template <class D, class... Os>
      requires(conv_traits<conv_impl<D, std::tuple<Os...>>>::applicable)
  using add_convention = facade_builder_impl<add_conv_t<
      Cs, conv_impl<D, std::tuple<Os...>>>, Rs, C>;
  template <class R> requires(is_meta_is_direct_well_formed<R>())
  using add_reflection = facade_builder_impl<Cs, add_tuple_t<Rs, R>, C>;
  template <facade F>
  using add_facade = facade_builder_impl<
      merge_conv_tuple_t<Cs, typename F::convention_types>,
      merge_tuple_t<Rs, typename F::reflection_types>,
      merge_constraints(C, F::constraints)>;
  template <std::size_t PtrSize, std::size_t PtrAlign =
      std::min(PtrSize, alignof(std::max_align_t))>
      requires(std::has_single_bit(PtrAlign) && PtrSize % PtrAlign == 0u)
  using restrict_layout = facade_builder_impl<
      Cs, Rs, make_restricted_layout(C, PtrSize, PtrAlign)>;
  template <constraint_level CL>
  using support_copy = facade_builder_impl<Cs, Rs, make_copyable(C, CL)>;
  template <constraint_level CL>
  using support_relocation = facade_builder_impl<
      Cs, Rs, make_relocatable(C, CL)>;
  template <constraint_level CL>
  using support_destruction = facade_builder_impl<
      Cs, Rs, make_destructible(C, CL)>;
  using build = facade_impl<Cs, Rs, normalize(C)>;
};

}  // namespace details

using facade_builder = details::facade_builder_impl<std::tuple<>, std::tuple<>,
    proxiable_ptr_constraints{
        .max_size = details::invalid_size,
        .max_align = details::invalid_size,
        .copyability = details::invalid_cl,
        .relocatability = details::invalid_cl,
        .destructibility = details::invalid_cl}>;

#define ___PRO_ACCESSOR_FUNC_IMPL(__D, __P, __QP, __SELF, __ARGS) \
    noexcept(noexcept(::pro::proxy_invoke<__D>( \
        ::std::declval<__QP>(), __ARGS))) \
    requires(requires { ::pro::proxy_invoke<__D>( \
        ::std::declval<__QP>(), __ARGS); }) \
    { return ::pro::proxy_invoke<__D>( \
        ::pro::access_proxy<__P>(__SELF), __ARGS); }

#define ___PRO_GEN_MEM_ACCESSOR_0(__SELF, __DECL, __D, __P, __ARGS, ...) \
    __DECL(, __VA_ARGS__) \
        ___PRO_ACCESSOR_FUNC_IMPL(__D, __P, const __P&, *this, __ARGS)
#define ___PRO_GEN_MEM_ACCESSOR_1(__SELF, __DECL, __D, __P, __ARGS, ...) \
    __DECL(&, __VA_ARGS__) \
        ___PRO_ACCESSOR_FUNC_IMPL(__D, __P, __P&, *this, __ARGS) \
    __DECL(const&, __VA_ARGS__) \
        ___PRO_ACCESSOR_FUNC_IMPL(__D, __P, const __P&, *this, __ARGS) \
    __DECL(&&, __VA_ARGS__) \
        ___PRO_ACCESSOR_FUNC_IMPL(__D, __P, __P&&, \
            ::std::forward<__SELF>(*this), __ARGS) \
    __DECL(const&&, __VA_ARGS__) \
        ___PRO_ACCESSOR_FUNC_IMPL(__D, __P, const __P&&, \
            ::std::forward<const __SELF>(*this), __ARGS) \

#define ___PRO_GEN_FREE_ACCESSOR_0(__SELF, __DECL, __D, __P, __ARGS, ...) \
    __DECL(__SELF& __self, __VA_ARGS__) \
        ___PRO_ACCESSOR_FUNC_IMPL(__D, __P, const __P&, __self, __ARGS)
#define ___PRO_GEN_FREE_ACCESSOR_1(__SELF, __DECL, __D, __P, __ARGS, ...) \
    __DECL(__SELF& __self, __VA_ARGS__) \
        ___PRO_ACCESSOR_FUNC_IMPL(__D, __P, __P&, __self, __ARGS) \
    __DECL(const __SELF& __self, __VA_ARGS__) \
        ___PRO_ACCESSOR_FUNC_IMPL(__D, __P, const __P&, __self, __ARGS) \
    __DECL(__SELF&& __self, __VA_ARGS__) \
        ___PRO_ACCESSOR_FUNC_IMPL(__D, __P, __P&&, \
            ::std::forward<__SELF>(__self), __ARGS) \
    __DECL(const __SELF&& __self, __VA_ARGS__) \
        ___PRO_ACCESSOR_FUNC_IMPL(__D, __P, const __P&&, \
            ::std::forward<const __SELF>(__self), __ARGS) \

#define ___PRO_DECL_MEM_ACCESSOR(__Q, __FNAME) \
    template <class... __Args> decltype(auto) __FNAME(__Args&&... __args) __Q

#define ___PRO_DECL_FREE_ACCESSOR(__SELF, __FNAME) \
    template <class... __Args> \
    friend decltype(auto) __FNAME(__SELF, __Args&&... __args)

namespace details {

template <int IS_DIRECT>
struct dispatch_base
    { static constexpr bool is_direct = static_cast<bool>(IS_DIRECT); };

template <std::size_t N>
struct sign {
  consteval sign(const char (&str)[N])
      { for (std::size_t i = 0; i < N; ++i) { value[i] = str[i]; } }

  char value[N];
};
template <std::size_t N>
sign(const char (&str)[N]) -> sign<N>;

enum class sign_pos_type { none, left, right };

template <sign SIGN, sign_pos_type POS>
struct op_dispatch_traits_impl : inapplicable_traits {};

#define ___PRO_OPERATOR_DISPATCH_TRAITS_PREFIX_0_IMPL(...) \
    template <> \
    struct op_dispatch_traits_impl<#__VA_ARGS__, sign_pos_type::left> \
        : applicable_traits { \
      struct base { \
        template <class T> \
        decltype(auto) operator()(T&& self) \
            ___PRO_DIRECT_FUNC_IMPL(__VA_ARGS__ std::forward<T>(self)) \
      }; \
      template <class D, class P> \
      struct indirect_accessor { \
        template <class Barrier = void> \
        decltype(auto) operator __VA_ARGS__ () \
            noexcept(noexcept( \
                proxy_invoke<lazy_eval_t<D>>(std::declval<const P&>()))) \
            requires(requires { proxy_invoke< \
                lazy_eval_t<D, Barrier>>(std::declval<const P&>()); }) \
            { return proxy_invoke<D>(access_proxy<P>(*this)); } \
      }; \
      template <class D, class P> \
      struct direct_accessor { \
        template <class Barrier = void> \
        decltype(auto) operator __VA_ARGS__ () & \
            noexcept(noexcept(proxy_invoke<D>(std::declval<P&>()))) \
            requires(requires { proxy_invoke< \
                lazy_eval_t<D, Barrier>>(std::declval<P&>()); }) \
            { return proxy_invoke<D>(access_proxy<P>(*this)); } \
        template <class Barrier = void> \
        decltype(auto) operator __VA_ARGS__ () const& \
            noexcept(noexcept(proxy_invoke<D>(std::declval<const P&>()))) \
            requires(requires { proxy_invoke< \
                lazy_eval_t<D, Barrier>>(std::declval<const P&>()); }) \
            { return proxy_invoke<D>(access_proxy<P>(*this)); } \
        template <class Barrier = void> \
        decltype(auto) operator __VA_ARGS__ () && \
            noexcept(noexcept(proxy_invoke<D>(std::declval<P&&>()))) \
            requires(requires { proxy_invoke< \
                lazy_eval_t<D, Barrier>>(std::declval<P&&>()); }) \
            { return proxy_invoke<D>(access_proxy<P>( \
                std::forward<direct_accessor>(*this))); } \
        template <class Barrier = void> \
        decltype(auto) operator __VA_ARGS__ () const&& \
            noexcept(noexcept(proxy_invoke<D>(std::declval<const P&&>()))) \
            requires(requires { proxy_invoke< \
                lazy_eval_t<D, Barrier>>(std::declval<const P&&>()); }) \
            { return proxy_invoke<D>(access_proxy<P>( \
                std::forward<const direct_accessor>(*this))); } \
      }; \
    };

#define ___PRO_DECL_OPERATOR_ACCESSOR_PREFIX_1(SELF, ...) \
    template <class Arg> \
    friend decltype(auto) operator __VA_ARGS__ (Arg&& arg, SELF)
#define ___PRO_OPERATOR_DISPATCH_TRAITS_PREFIX_1_IMPL(...) \
    template <> \
    struct op_dispatch_traits_impl<#__VA_ARGS__, sign_pos_type::left> \
        : applicable_traits { \
      struct base { \
        template <class T, class Arg> \
        decltype(auto) operator()(T&& self, Arg&& arg) \
            ___PRO_DIRECT_FUNC_IMPL(std::forward<Arg>(arg) __VA_ARGS__ \
                std::forward<T>(self)) \
      }; \
      template <class D, class P> \
      struct indirect_accessor { \
        ___PRO_GEN_FREE_ACCESSOR_0( \
            indirect_accessor, ___PRO_DECL_OPERATOR_ACCESSOR_PREFIX_1, D, P, \
                std::forward<Arg>(arg), __VA_ARGS__) \
      }; \
      template <class D, class P> \
      struct direct_accessor { \
        ___PRO_GEN_FREE_ACCESSOR_1( \
            direct_accessor, ___PRO_DECL_OPERATOR_ACCESSOR_PREFIX_1, D, P, \
                std::forward<Arg>(arg), __VA_ARGS__) \
      }; \
    };

#define ___PRO_OPERATOR_DISPATCH_TRAITS_PREFIX_0_OR_1_IMPL(...) \
    template <> \
    struct op_dispatch_traits_impl<#__VA_ARGS__, sign_pos_type::left> \
        : applicable_traits { \
      struct base { \
        template <class T> \
        decltype(auto) operator()(T&& self) \
            ___PRO_DIRECT_FUNC_IMPL(__VA_ARGS__ std::forward<T>(self)) \
        template <class T, class Arg> \
        decltype(auto) operator()(T&& self, Arg&& arg) \
            ___PRO_DIRECT_FUNC_IMPL(std::forward<Arg>(arg) __VA_ARGS__ \
                std::forward<T>(self)) \
      }; \
      template <class D, class P> \
      struct indirect_accessor { \
        template <class Barrier = void> \
        decltype(auto) operator __VA_ARGS__ () \
            noexcept(noexcept(proxy_invoke<D>(std::declval<const P&>()))) \
            requires(requires { proxy_invoke< \
                lazy_eval_t<D, Barrier>>(std::declval<const P&>()); }) \
            { return proxy_invoke<D>(access_proxy<P>(*this)); } \
        ___PRO_GEN_FREE_ACCESSOR_0( \
            indirect_accessor, ___PRO_DECL_OPERATOR_ACCESSOR_PREFIX_1, D, P, \
                std::forward<Arg>(arg), __VA_ARGS__) \
      }; \
      template <class D, class P> \
      struct direct_accessor { \
        template <class Barrier = void> \
        decltype(auto) operator __VA_ARGS__ () & \
            noexcept(noexcept(proxy_invoke<D>(std::declval<P&>()))) \
            requires(requires { proxy_invoke< \
                lazy_eval_t<D, Barrier>>(std::declval<P&>()); }) \
            { return proxy_invoke<D>(access_proxy<P>(*this)); } \
        template <class Barrier = void> \
        decltype(auto) operator __VA_ARGS__ () const& \
            noexcept(noexcept(proxy_invoke<D>(std::declval<const P&>()))) \
            requires(requires { proxy_invoke< \
                lazy_eval_t<D, Barrier>>(std::declval<const P&>()); }) \
            { return proxy_invoke<D>(access_proxy<P>(*this)); } \
        template <class Barrier = void> \
        decltype(auto) operator __VA_ARGS__ () && \
            noexcept(noexcept(proxy_invoke<D>(std::declval<P&&>()))) \
            requires(requires { proxy_invoke< \
                lazy_eval_t<D, Barrier>>(std::declval<P&&>()); }) \
            { return proxy_invoke<D>(access_proxy<P>( \
                std::forward<direct_accessor>(*this))); } \
        template <class Barrier = void> \
        decltype(auto) operator __VA_ARGS__ () const&& \
            noexcept(noexcept(proxy_invoke<D>(std::declval<const P&&>()))) \
            requires(requires { proxy_invoke< \
                lazy_eval_t<D, Barrier>>(std::declval<const P&&>()); }) \
            { return proxy_invoke<D>(access_proxy<P>( \
                std::forward<const direct_accessor>(*this))); } \
        ___PRO_GEN_FREE_ACCESSOR_1( \
            direct_accessor, ___PRO_DECL_OPERATOR_ACCESSOR_PREFIX_1, D, P, \
                std::forward<Arg>(arg), __VA_ARGS__) \
      }; \
    };

#define ___PRO_OPERATOR_DISPATCH_TRAITS_POSTFIX_0_IMPL(...) \
    template <> \
    struct op_dispatch_traits_impl<#__VA_ARGS__, sign_pos_type::right> \
        : applicable_traits { \
      struct base { \
        template <class T> \
        decltype(auto) operator()(T&& self) \
            ___PRO_DIRECT_FUNC_IMPL(std::forward<T>(self) __VA_ARGS__) \
      }; \
      template <class D, class P> \
      struct indirect_accessor { \
        template <class Barrier = void> \
        decltype(auto) operator __VA_ARGS__ (int) \
            noexcept(noexcept(proxy_invoke<D>(std::declval<const P&>()))) \
            requires(requires { proxy_invoke< \
                lazy_eval_t<D, Barrier>>(std::declval<const P&>()); }) \
            { return proxy_invoke<D>(access_proxy<P>(*this)); } \
      }; \
      template <class D, class P> \
      struct direct_accessor { \
        template <class Barrier = void> \
        decltype(auto) operator __VA_ARGS__ (int) & \
            noexcept(noexcept(proxy_invoke<D>(std::declval<P&>()))) \
            requires(requires { proxy_invoke< \
                lazy_eval_t<D, Barrier>>(std::declval<P&>()); }) \
            { return proxy_invoke<D>(access_proxy<P>(*this)); } \
        template <class Barrier = void> \
        decltype(auto) operator __VA_ARGS__ (int) const& \
            noexcept(noexcept(proxy_invoke<D>(std::declval<const P&>()))) \
            requires(requires { proxy_invoke< \
                lazy_eval_t<D, Barrier>>(std::declval<const P&>()); }) \
            { return proxy_invoke<D>(access_proxy<P>(*this)); } \
        template <class Barrier = void> \
        decltype(auto) operator __VA_ARGS__ (int) && \
            noexcept(noexcept(proxy_invoke<D>(std::declval<P&&>()))) \
            requires(requires { proxy_invoke< \
                lazy_eval_t<D, Barrier>>(std::declval<P&&>()); }) \
            { return proxy_invoke<D>(access_proxy<P>( \
                std::forward<direct_accessor>(*this))); } \
        template <class Barrier = void> \
        decltype(auto) operator __VA_ARGS__ (int) const&& \
            noexcept(noexcept(proxy_invoke<D>(std::declval<const P&&>()))) \
            requires(requires { proxy_invoke< \
                lazy_eval_t<D, Barrier>>(std::declval<const P&&>()); }) \
            { return proxy_invoke<D>(access_proxy<P>( \
                std::forward<const direct_accessor>(*this))); } \
      }; \
    };

#define ___PRO_DECL_OPERATOR_ACCESSOR_POSTFIX_1(Q, ...) \
    template <class Arg> \
    decltype(auto) operator __VA_ARGS__ (Arg&& arg) Q
#define ___PRO_OPERATOR_DISPATCH_TRAITS_POSTFIX_1_IMPL(...) \
    template <> \
    struct op_dispatch_traits_impl<#__VA_ARGS__, sign_pos_type::right> \
        : applicable_traits { \
      struct base { \
        template <class T, class Arg> \
        decltype(auto) operator()(T&& self, Arg&& arg) \
            ___PRO_DIRECT_FUNC_IMPL(std::forward<T>(self) __VA_ARGS__ \
                std::forward<Arg>(arg)) \
      }; \
      template <class D, class P> \
      struct indirect_accessor { \
        ___PRO_GEN_MEM_ACCESSOR_0( \
            indirect_accessor, ___PRO_DECL_OPERATOR_ACCESSOR_POSTFIX_1, D, P, \
                std::forward<Arg>(arg), __VA_ARGS__) \
      }; \
      template <class D, class P> \
      struct direct_accessor { \
        ___PRO_GEN_MEM_ACCESSOR_1( \
            direct_accessor, ___PRO_DECL_OPERATOR_ACCESSOR_POSTFIX_1, D, P, \
                std::forward<Arg>(arg), __VA_ARGS__) \
      }; \
    };

#define ___PRO_OPERATOR_DISPATCH_TRAITS_ASSIGNMENT_IMPL(...) \
    template <> \
    struct op_dispatch_traits_impl<#__VA_ARGS__, sign_pos_type::right> \
        : applicable_traits { \
      struct base { \
        template <class Arg> \
        decltype(auto) operator()(auto& self, Arg&& arg) \
            ___PRO_DIRECT_FUNC_IMPL(self __VA_ARGS__ std::forward<Arg>(arg)) \
      }; \
      template <class D, class P> \
      struct indirect_accessor { \
        template <class Arg> \
        decltype(auto) operator __VA_ARGS__ (Arg&& arg) \
            noexcept(noexcept(proxy_invoke<D>(std::declval<const P&>(), \
                std::forward<Arg>(arg)))) \
            requires(requires { proxy_invoke<D>(std::declval<const P&>(), \
                std::forward<Arg>(arg)); }) { \
          proxy_invoke<D>(access_proxy<P>(*this), std::forward<Arg>(arg)); \
          return *access_proxy<P>(*this); \
        } \
      }; \
      template <class D, class P> \
      struct direct_accessor { \
        template <class Arg> \
        decltype(auto) operator __VA_ARGS__ (Arg&& arg) \
            noexcept(noexcept(proxy_invoke<D>(std::declval<P&>(), \
                std::forward<Arg>(arg)))) \
            requires(requires { proxy_invoke<D>(std::declval<P&>(), \
                std::forward<Arg>(arg)); }) { \
          proxy_invoke<D>(access_proxy<P>(*this), std::forward<Arg>(arg)); \
          return access_proxy<P>(*this); \
        } \
      }; \
    };

#define ___PRO_OPERATOR_DISPATCH_TRAITS_BINARY_IMPL(...) \
    ___PRO_OPERATOR_DISPATCH_TRAITS_PREFIX_1_IMPL(__VA_ARGS__) \
    ___PRO_OPERATOR_DISPATCH_TRAITS_POSTFIX_1_IMPL(__VA_ARGS__)

#define ___PRO_OPERATOR_DISPATCH_TRAITS_EXTENDED_BINARY_IMPL(...) \
    ___PRO_OPERATOR_DISPATCH_TRAITS_PREFIX_0_OR_1_IMPL(__VA_ARGS__) \
    ___PRO_OPERATOR_DISPATCH_TRAITS_POSTFIX_1_IMPL(__VA_ARGS__)

___PRO_OPERATOR_DISPATCH_TRAITS_EXTENDED_BINARY_IMPL(+)
___PRO_OPERATOR_DISPATCH_TRAITS_EXTENDED_BINARY_IMPL(-)
___PRO_OPERATOR_DISPATCH_TRAITS_EXTENDED_BINARY_IMPL(*)
___PRO_OPERATOR_DISPATCH_TRAITS_BINARY_IMPL(/)
___PRO_OPERATOR_DISPATCH_TRAITS_BINARY_IMPL(%)
___PRO_OPERATOR_DISPATCH_TRAITS_PREFIX_0_IMPL(++)
___PRO_OPERATOR_DISPATCH_TRAITS_POSTFIX_0_IMPL(++)
___PRO_OPERATOR_DISPATCH_TRAITS_PREFIX_0_IMPL(--)
___PRO_OPERATOR_DISPATCH_TRAITS_POSTFIX_0_IMPL(--)
___PRO_OPERATOR_DISPATCH_TRAITS_BINARY_IMPL(==)
___PRO_OPERATOR_DISPATCH_TRAITS_BINARY_IMPL(!=)
___PRO_OPERATOR_DISPATCH_TRAITS_BINARY_IMPL(>)
___PRO_OPERATOR_DISPATCH_TRAITS_BINARY_IMPL(<)
___PRO_OPERATOR_DISPATCH_TRAITS_BINARY_IMPL(>=)
___PRO_OPERATOR_DISPATCH_TRAITS_BINARY_IMPL(<=)
___PRO_OPERATOR_DISPATCH_TRAITS_BINARY_IMPL(<=>)
___PRO_OPERATOR_DISPATCH_TRAITS_PREFIX_0_IMPL(!)
___PRO_OPERATOR_DISPATCH_TRAITS_BINARY_IMPL(&&)
___PRO_OPERATOR_DISPATCH_TRAITS_BINARY_IMPL(||)
___PRO_OPERATOR_DISPATCH_TRAITS_PREFIX_0_IMPL(~)
___PRO_OPERATOR_DISPATCH_TRAITS_EXTENDED_BINARY_IMPL(&)
___PRO_OPERATOR_DISPATCH_TRAITS_BINARY_IMPL(|)
___PRO_OPERATOR_DISPATCH_TRAITS_BINARY_IMPL(^)
___PRO_OPERATOR_DISPATCH_TRAITS_BINARY_IMPL(<<)
___PRO_OPERATOR_DISPATCH_TRAITS_BINARY_IMPL(>>)
___PRO_OPERATOR_DISPATCH_TRAITS_ASSIGNMENT_IMPL(+=)
___PRO_OPERATOR_DISPATCH_TRAITS_ASSIGNMENT_IMPL(-=)
___PRO_OPERATOR_DISPATCH_TRAITS_ASSIGNMENT_IMPL(*=)
___PRO_OPERATOR_DISPATCH_TRAITS_ASSIGNMENT_IMPL(/=)
___PRO_OPERATOR_DISPATCH_TRAITS_ASSIGNMENT_IMPL(&=)
___PRO_OPERATOR_DISPATCH_TRAITS_ASSIGNMENT_IMPL(|=)
___PRO_OPERATOR_DISPATCH_TRAITS_ASSIGNMENT_IMPL(^=)
___PRO_OPERATOR_DISPATCH_TRAITS_ASSIGNMENT_IMPL(<<=)
___PRO_OPERATOR_DISPATCH_TRAITS_ASSIGNMENT_IMPL(>>=)
___PRO_OPERATOR_DISPATCH_TRAITS_BINARY_IMPL(,)
___PRO_OPERATOR_DISPATCH_TRAITS_BINARY_IMPL(->*)

#undef ___PRO_OPERATOR_DISPATCH_TRAITS_EXTENDED_BINARY_IMPL
#undef ___PRO_OPERATOR_DISPATCH_TRAITS_BINARY_IMPL
#undef ___PRO_OPERATOR_DISPATCH_TRAITS_ASSIGNMENT_IMPL
#undef ___PRO_OPERATOR_DISPATCH_TRAITS_POSTFIX_1_IMPL
#undef ___PRO_DECL_OPERATOR_ACCESSOR_POSTFIX_1
#undef ___PRO_OPERATOR_DISPATCH_TRAITS_POSTFIX_0_IMPL
#undef ___PRO_OPERATOR_DISPATCH_TRAITS_PREFIX_0_OR_1_IMPL
#undef ___PRO_OPERATOR_DISPATCH_TRAITS_PREFIX_1_IMPL
#undef ___PRO_DECL_OPERATOR_ACCESSOR_PREFIX_1
#undef ___PRO_OPERATOR_DISPATCH_TRAITS_PREFIX_0_IMPL

template <>
struct op_dispatch_traits_impl<"()", sign_pos_type::right>
    : applicable_traits {
  struct base {
    template <class T, class... Args>
    decltype(auto) operator()(T&& self, Args&&... args)
        ___PRO_DIRECT_FUNC_IMPL(
            std::forward<T>(self)(std::forward<Args>(args)...))
  };
  template <class D, class P>
  struct indirect_accessor {
      ___PRO_GEN_MEM_ACCESSOR_0(
          indirect_accessor, ___PRO_DECL_MEM_ACCESSOR, D, P,
              std::forward<__Args>(__args)..., operator())
  };
  template <class D, class P>
  struct direct_accessor {
      ___PRO_GEN_MEM_ACCESSOR_1(
          direct_accessor, ___PRO_DECL_MEM_ACCESSOR, D, P,
              std::forward<__Args>(__args)..., operator())
  };
};
template <>
struct op_dispatch_traits_impl<"[]", sign_pos_type::right>
    : applicable_traits {
  struct base {
#if defined(__cpp_multidimensional_subscript) && __cpp_multidimensional_subscript >= 202110L
    template <class T, class... Args>
    decltype(auto) operator()(T&& self, Args&&... args)
        ___PRO_DIRECT_FUNC_IMPL(
            std::forward<T>(self)[std::forward<Args>(args)...])
#else
    template <class T, class Arg>
    decltype(auto) operator()(T&& self, Arg&& arg)
        ___PRO_DIRECT_FUNC_IMPL(std::forward<T>(self)[std::forward<Arg>(arg)])
#endif  // defined(__cpp_multidimensional_subscript) && __cpp_multidimensional_subscript >= 202110L
  };
  template <class D, class P>
  struct indirect_accessor {
      ___PRO_GEN_MEM_ACCESSOR_0(
          indirect_accessor, ___PRO_DECL_MEM_ACCESSOR, D, P,
              std::forward<__Args>(__args)..., operator[])
  };
  template <class D, class P>
  struct direct_accessor {
      ___PRO_GEN_MEM_ACCESSOR_1(
          direct_accessor, ___PRO_DECL_MEM_ACCESSOR, D, P,
              std::forward<__Args>(__args)..., operator[])
  };
};

template <sign SIGN, sign_pos_type POS>
struct op_dispatch_traits : op_dispatch_traits_impl<SIGN, POS>
    { static_assert(op_dispatch_traits::applicable, "Unexpected operator"); };
template <sign SIGN>
    requires(op_dispatch_traits_impl<SIGN, sign_pos_type::right>::applicable)
struct op_dispatch_traits<SIGN, sign_pos_type::none>
    : op_dispatch_traits_impl<SIGN, sign_pos_type::right> {};
template <sign SIGN>
    requires(!op_dispatch_traits_impl<SIGN, sign_pos_type::right>::applicable &&
        op_dispatch_traits_impl<SIGN, sign_pos_type::left>::applicable)
struct op_dispatch_traits<SIGN, sign_pos_type::none>
    : op_dispatch_traits_impl<SIGN, sign_pos_type::left> {};

template <int IS_DIRECT, sign SIGN, sign_pos_type POS>
struct op_dispatch_base :
    dispatch_base<IS_DIRECT>, op_dispatch_traits<SIGN, POS>::base {};
template <int IS_DIRECT, sign SIGN, sign_pos_type POS, class D, class P>
using op_dispatch_accessor = std::conditional_t<static_cast<bool>(IS_DIRECT),
    typename op_dispatch_traits<SIGN, POS>::template direct_accessor<D, P>,
    typename op_dispatch_traits<SIGN, POS>::template indirect_accessor<D, P>>;

template <class T>
struct conversion_dispatch_base_impl {
  template <class U>
  T operator()(U&& self)
      ___PRO_DIRECT_FUNC_IMPL(static_cast<T>(std::forward<U>(self)))
};
template <int IS_DIRECT, class T>
struct conversion_dispatch_base
    : dispatch_base<IS_DIRECT>, conversion_dispatch_base_impl<T> {};
template <class T, class D, class P>
struct indirect_conversion_accessor {
  template <class Barrier = void>
  explicit operator T()
      noexcept(noexcept(proxy_invoke<D>(std::declval<const P&>())))
      requires(requires { proxy_invoke<
          lazy_eval_t<D, Barrier>>(std::declval<const P&>()); })
      { return proxy_invoke<D>(access_proxy<P>(*this)); }
};
template <class T, class D, class P>
struct direct_conversion_accessor {
  static_assert(std::is_nothrow_default_constructible_v<T>);
  template <class Barrier = void>
  explicit operator T() &
      noexcept(noexcept(proxy_invoke<D>(std::declval<P&>())))
      requires(requires { proxy_invoke<
          lazy_eval_t<D, Barrier>>(std::declval<P&>()); }) {
    if (access_proxy<P>(*this).has_value()) {
      return proxy_invoke<D>(access_proxy<P>(*this));
    } else {
      return T{};
    }
  }
  template <class Barrier = void>
  explicit operator T() const&
      noexcept(noexcept(proxy_invoke<D>(std::declval<const P&>())))
      requires(requires { proxy_invoke<
          lazy_eval_t<D, Barrier>>(std::declval<const P&>()); }) {
    if (access_proxy<P>(*this).has_value()) {
      return proxy_invoke<D>(access_proxy<P>(*this));
    } else {
      return T{};
    }
  }
  template <class Barrier = void>
  explicit operator T() &&
      noexcept(noexcept(proxy_invoke<D>(std::declval<P&&>())))
      requires(requires { proxy_invoke<
          lazy_eval_t<D, Barrier>>(std::declval<P&&>()); }) {
    if (access_proxy<P>(*this).has_value()) {
      return proxy_invoke<D>(access_proxy<P>(std::forward<
          direct_conversion_accessor>(*this)));
    } else {
      return T{};
    }
  }
  template <class Barrier = void>
  explicit operator T() const&&
      noexcept(noexcept(proxy_invoke<D>(std::declval<const P&&>())))
      requires(requires { proxy_invoke<
          lazy_eval_t<D, Barrier>>(std::declval<const P&&>()); }) {
    if (access_proxy<P>(*this).has_value()) {
      return proxy_invoke<D>(access_proxy<P>(std::forward<
          const direct_conversion_accessor>(*this)));
    } else {
      return T{};
    }
  }
};
template <int IS_DIRECT, class T, class D, class P>
using conversion_dispatch_accessor = std::conditional_t<
    static_cast<bool>(IS_DIRECT), direct_conversion_accessor<T, D, P>,
    indirect_conversion_accessor<T, D, P>>;

}  // namespace details

#define ___PRO_EXPAND_IMPL(__X) __X
#define ___PRO_EXPAND_MACRO_IMPL( \
    __MACRO, __1, __2, __3, __4, __5, __NAME, ...) \
    __MACRO##_##__NAME
#define ___PRO_EXPAND_MACRO(__MACRO, ...) \
    ___PRO_EXPAND_IMPL(___PRO_EXPAND_MACRO_IMPL( \
        __MACRO, __VA_ARGS__, 5, 4, 3)(__VA_ARGS__))

#define ___PRO_DEFAULT_DISPATCH_CALL_IMPL(__DEFFUNC) \
    template <class... __Args> \
    decltype(auto) operator()(::std::nullptr_t, __Args&&... __args) \
        ___PRO_DIRECT_FUNC_IMPL(__DEFFUNC(::std::forward<__Args>(__args)...))

#define ___PRO_DEF_MEM_DISPATCH_IMPL( \
    __NAME, __IS_DIRECT, __FUNC, __FNAME, ...) \
    struct __NAME : ::pro::details::dispatch_base<__IS_DIRECT> { \
      using __name = __NAME; \
      template <class __T, class... __Args> \
      decltype(auto) operator()(__T&& __self, __Args&&... __args) \
          ___PRO_DIRECT_FUNC_IMPL(::std::forward<__T>(__self) \
              .__FUNC(::std::forward<__Args>(__args)...)) \
      template <class __P> \
      struct accessor { \
        ___PRO_GEN_MEM_ACCESSOR_##__IS_DIRECT( \
            accessor, ___PRO_DECL_MEM_ACCESSOR, __name, __P, \
                ::std::forward<__Args>(__args)..., __FNAME) \
      }; \
      __VA_ARGS__ \
    }
#define ___PRO_DEF_MEM_DISPATCH_3(__NAME, __IS_DIRECT, __FUNC) \
    ___PRO_DEF_MEM_DISPATCH_IMPL(__NAME, __IS_DIRECT, __FUNC, __FUNC)
#define ___PRO_DEF_MEM_DISPATCH_4(__NAME, __IS_DIRECT, __FUNC, __FNAME) \
    ___PRO_DEF_MEM_DISPATCH_IMPL(__NAME, __IS_DIRECT, __FUNC, __FNAME)
#define ___PRO_DEF_MEM_DISPATCH_5( \
    __NAME, __IS_DIRECT, __FUNC, __FNAME, __DEFFUNC) \
    ___PRO_DEF_MEM_DISPATCH_IMPL(__NAME, __IS_DIRECT, __FUNC, __FNAME, \
        ___PRO_DEFAULT_DISPATCH_CALL_IMPL(__DEFFUNC))
#define PRO_DEF_INDIRECT_MEM_DISPATCH(__NAME, ...) \
    ___PRO_EXPAND_MACRO(___PRO_DEF_MEM_DISPATCH, __NAME, 0, __VA_ARGS__)
#define PRO_DEF_DIRECT_MEM_DISPATCH(__NAME, ...) \
    ___PRO_EXPAND_MACRO(___PRO_DEF_MEM_DISPATCH, __NAME, 1, __VA_ARGS__)
#define PRO_DEF_MEM_DISPATCH(__NAME, ...) \
    PRO_DEF_INDIRECT_MEM_DISPATCH(__NAME, __VA_ARGS__)

#define ___PRO_DEF_FREE_DISPATCH_IMPL( \
    __NAME, __IS_DIRECT, __FUNC, __FNAME, ...) \
    struct __NAME : ::pro::details::dispatch_base<__IS_DIRECT> { \
      using __name = __NAME; \
      template <class __T, class... __Args> \
      decltype(auto) operator()(__T&& __self, __Args&&... __args) \
          ___PRO_DIRECT_FUNC_IMPL(__FUNC(::std::forward<__T>(__self), \
              ::std::forward<__Args>(__args)...)) \
      template <class __P> \
      struct accessor { \
        ___PRO_GEN_FREE_ACCESSOR_##__IS_DIRECT( \
            accessor, ___PRO_DECL_FREE_ACCESSOR, __name, __P, \
                ::std::forward<__Args>(__args)..., __FNAME) \
      }; \
      __VA_ARGS__ \
    }
#define ___PRO_DEF_FREE_DISPATCH_3(__NAME, __IS_DIRECT, __FUNC) \
    ___PRO_DEF_FREE_DISPATCH_IMPL(__NAME, __IS_DIRECT, __FUNC, __FUNC)
#define ___PRO_DEF_FREE_DISPATCH_4(__NAME, __IS_DIRECT, __FUNC, __FNAME) \
    ___PRO_DEF_FREE_DISPATCH_IMPL(__NAME, __IS_DIRECT, __FUNC, __FNAME)
#define ___PRO_DEF_FREE_DISPATCH_5( \
    __NAME, __IS_DIRECT, __FUNC, __FNAME, __DEFFUNC) \
    ___PRO_DEF_FREE_DISPATCH_IMPL(__NAME, __IS_DIRECT, __FUNC, __FNAME, \
        ___PRO_DEFAULT_DISPATCH_CALL_IMPL(__DEFFUNC))
#define PRO_DEF_INDIRECT_FREE_DISPATCH(__NAME, ...) \
    ___PRO_EXPAND_MACRO(___PRO_DEF_FREE_DISPATCH, __NAME, 0, __VA_ARGS__)
#define PRO_DEF_DIRECT_FREE_DISPATCH(__NAME, ...) \
    ___PRO_EXPAND_MACRO(___PRO_DEF_FREE_DISPATCH, __NAME, 1, __VA_ARGS__)
#define PRO_DEF_FREE_DISPATCH(__NAME, ...) \
    PRO_DEF_INDIRECT_FREE_DISPATCH(__NAME, __VA_ARGS__)

#define ___PRO_DEF_OPERATOR_DISPATCH_IMPL( \
    __NAME, __IS_DIRECT, __POS, __SIGN, ...) \
    struct __NAME : ::pro::details::op_dispatch_base<__IS_DIRECT, __SIGN, \
        ::pro::details::sign_pos_type::__POS> { \
      using ::pro::details::op_dispatch_base<__IS_DIRECT, __SIGN, \
          ::pro::details::sign_pos_type::__POS>::operator(); \
      template <class __P> \
      using accessor = ::pro::details::op_dispatch_accessor<__IS_DIRECT, \
          __SIGN, ::pro::details::sign_pos_type::__POS, __NAME, __P>; \
      __VA_ARGS__ \
    };
#define ___PRO_DEF_OPERATOR_DISPATCH_4(__NAME, __IS_DIRECT, __POS, __SIGN) \
    ___PRO_DEF_OPERATOR_DISPATCH_IMPL(__NAME, __IS_DIRECT, __POS, __SIGN)
#define ___PRO_DEF_OPERATOR_DISPATCH_5( \
    __NAME, __IS_DIRECT, __POS, __SIGN, __DEFFUNC) \
    ___PRO_DEF_OPERATOR_DISPATCH_IMPL(__NAME, __IS_DIRECT, __POS, __SIGN, \
        ___PRO_DEFAULT_DISPATCH_CALL_IMPL(__DEFFUNC))
#define PRO_DEF_INDIRECT_OPERATOR_DISPATCH(__NAME, ...) \
    ___PRO_EXPAND_MACRO(___PRO_DEF_OPERATOR_DISPATCH, __NAME, 0, none, \
        __VA_ARGS__)
#define PRO_DEF_DIRECT_OPERATOR_DISPATCH(__NAME, ...) \
    ___PRO_EXPAND_MACRO(___PRO_DEF_OPERATOR_DISPATCH, __NAME, 1, none, \
        __VA_ARGS__)
#define PRO_DEF_OPERATOR_DISPATCH(__NAME, ...) \
    PRO_DEF_INDIRECT_OPERATOR_DISPATCH(__NAME, __VA_ARGS__)
#define PRO_DEF_INDIRECT_PREFIX_OPERATOR_DISPATCH(__NAME, ...) \
    ___PRO_EXPAND_MACRO(___PRO_DEF_OPERATOR_DISPATCH, __NAME, 0, left, \
        __VA_ARGS__)
#define PRO_DEF_DIRECT_PREFIX_OPERATOR_DISPATCH(__NAME, ...) \
    ___PRO_EXPAND_MACRO(___PRO_DEF_OPERATOR_DISPATCH, __NAME, 1, left, \
        __VA_ARGS__)
#define PRO_DEF_PREFIX_OPERATOR_DISPATCH(__NAME, ...) \
    PRO_DEF_INDIRECT_PREFIX_OPERATOR_DISPATCH(__NAME, __VA_ARGS__)
#define PRO_DEF_INDIRECT_POSTFIX_OPERATOR_DISPATCH(__NAME, ...) \
    ___PRO_EXPAND_MACRO(___PRO_DEF_OPERATOR_DISPATCH, __NAME, 0, right, \
        __VA_ARGS__)
#define PRO_DEF_DIRECT_POSTFIX_OPERATOR_DISPATCH(__NAME, ...) \
    ___PRO_EXPAND_MACRO(___PRO_DEF_OPERATOR_DISPATCH, __NAME, 1, right, \
        __VA_ARGS__)
#define PRO_DEF_POSTFIX_OPERATOR_DISPATCH(__NAME, ...) \
    PRO_DEF_INDIRECT_POSTFIX_OPERATOR_DISPATCH(__NAME, __VA_ARGS__)

#define ___PRO_DEF_CONVERSION_DISPATCH_IMPL(__NAME, __IS_DIRECT, __T, ...) \
    struct __NAME \
        : ::pro::details::conversion_dispatch_base<__IS_DIRECT, __T> { \
      using ::pro::details::conversion_dispatch_base<__IS_DIRECT, __T> \
          ::operator(); \
      template <class __P> \
      using accessor = ::pro::details::conversion_dispatch_accessor< \
          __IS_DIRECT, __T, __NAME, __P>; \
      __VA_ARGS__ \
    };
#define ___PRO_DEF_CONVERSION_DISPATCH_3(__NAME, __IS_DIRECT, __T) \
    ___PRO_DEF_CONVERSION_DISPATCH_IMPL(__NAME, __IS_DIRECT, __T)
#define ___PRO_DEF_CONVERSION_DISPATCH_4(__NAME, __IS_DIRECT, __T, __DEFFUNC) \
    ___PRO_DEF_CONVERSION_DISPATCH_IMPL( \
        __NAME, __IS_DIRECT, __T, ___PRO_DEFAULT_DISPATCH_CALL_IMPL(__DEFFUNC))
#define PRO_DEF_INDIRECT_CONVERSION_DISPATCH(__NAME, ...) \
    ___PRO_EXPAND_MACRO(___PRO_DEF_CONVERSION_DISPATCH, __NAME, 0, __VA_ARGS__)
#define PRO_DEF_DIRECT_CONVERSION_DISPATCH(__NAME, ...) \
    ___PRO_EXPAND_MACRO(___PRO_DEF_CONVERSION_DISPATCH, __NAME, 1, __VA_ARGS__)
#define PRO_DEF_CONVERSION_DISPATCH(__NAME, ...) \
    PRO_DEF_INDIRECT_CONVERSION_DISPATCH(__NAME, __VA_ARGS__)

}  // namespace pro

#undef ___PRO_NO_UNIQUE_ADDRESS_ATTRIBUTE

#endif  // _MSFT_PROXY_
