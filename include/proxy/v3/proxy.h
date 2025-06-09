// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#ifndef _MSFT_PROXY3_
#define _MSFT_PROXY3_

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <bit>
#include <concepts>
#include <exception>
#include <initializer_list>
#include <limits>
#include <memory>
#include <new>
#include <tuple>
#include <type_traits>
#include <utility>

#if __STDC_HOSTED__
#include <atomic>
#include <format>
#endif  // __STDC_HOSTED__

#if __cpp_rtti >= 199711L
#include <optional>
#include <typeinfo>
#endif  // __cpp_rtti >= 199711L

#if __has_cpp_attribute(msvc::no_unique_address)
#define ___PRO_NO_UNIQUE_ADDRESS_ATTRIBUTE msvc::no_unique_address
#elif __has_cpp_attribute(no_unique_address)
#define ___PRO_NO_UNIQUE_ADDRESS_ATTRIBUTE no_unique_address
#else
#error "Proxy requires C++20 attribute no_unique_address"
#endif

#if __cpp_exceptions >= 199711L
#define ___PRO_THROW(...) throw __VA_ARGS__
#else
#define ___PRO_THROW(...) std::abort()
#endif  // __cpp_exceptions >= 199711L

#if __cpp_static_call_operator >= 202207L
#define ___PRO3_STATIC_CALL(__R, ...) static __R operator()(__VA_ARGS__)
#else
#define ___PRO3_STATIC_CALL(__R, ...) __R operator()(__VA_ARGS__) const
#endif  // __cpp_static_call_operator >= 202207L

#ifdef _MSC_VER
#define ___PRO3_ENFORCE_EBO __declspec(empty_bases)
#else
#define ___PRO3_ENFORCE_EBO
#endif  // _MSC_VER

#ifdef NDEBUG
#define ___PRO3_DEBUG(...)
#else
#define ___PRO3_DEBUG(...) __VA_ARGS__
#endif  // NDEBUG

#define __msft_lib_proxy3 202505L

namespace pro::inline v3 {

namespace details {

struct applicable_traits { static constexpr bool applicable = true; };
struct inapplicable_traits { static constexpr bool applicable = false; };

template <template <class, class> class R, class O, class... Is>
struct recursive_reduction : std::type_identity<O> {};
template <template <class, class> class R, class O, class I, class... Is>
struct recursive_reduction<R, O, I, Is...>
    : recursive_reduction<R, R<O, I>, Is...> {};
template <template <class, class> class R, class O, class... Is>
using recursive_reduction_t = typename recursive_reduction<R, O, Is...>::type;

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
    : std::type_identity<T<Args..., std::tuple_element_t<Is, TL>...>> {};
template <template <class...> class T, class TL, class... Args>
using instantiated_t = typename instantiated_traits<
    T, TL, std::make_index_sequence<std::tuple_size_v<TL>>, Args...>::type;

template <class F> struct basic_facade_traits;

}  // namespace details

enum class constraint_level { none, nontrivial, nothrow, trivial };

struct proxiable_ptr_constraints {
  std::size_t max_size;
  std::size_t max_align;
  constraint_level copyability;
  constraint_level relocatability;
  constraint_level destructibility;
};

template <template <class> class O>
struct facade_aware_overload_t { facade_aware_overload_t() = delete; };

template <class F>
concept facade = details::basic_facade_traits<F>::applicable;

template <facade F> struct proxy_indirect_accessor;
template <facade F> class ___PRO3_ENFORCE_EBO proxy;

namespace details {

enum class qualifier_type { lv, const_lv, rv, const_rv };
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
template <class T, qualifier_type Q>
using add_qualifier_ptr_t = std::remove_reference_t<add_qualifier_t<T, Q>>*;

template <class T>
consteval bool has_copyability(constraint_level level) {
  switch (level) {
    case constraint_level::none: return true;
    case constraint_level::nontrivial: return std::is_copy_constructible_v<T>;
    case constraint_level::nothrow:
      return std::is_nothrow_copy_constructible_v<T>;
    case constraint_level::trivial:
      return std::is_trivially_copy_constructible_v<T> &&
          std::is_trivially_destructible_v<T>;
    default: return false;
  }
}
template <class T>
consteval bool has_relocatability(constraint_level level) {
  switch (level) {
    case constraint_level::none: return true;
    case constraint_level::nontrivial:
      return std::is_move_constructible_v<T> && std::is_destructible_v<T>;
    case constraint_level::nothrow:
      return std::is_nothrow_move_constructible_v<T> &&
          std::is_nothrow_destructible_v<T>;
    case constraint_level::trivial:
      return std::is_trivially_move_constructible_v<T> &&
          std::is_trivially_destructible_v<T>;
    default: return false;
  }
}
template <class T>
consteval bool has_destructibility(constraint_level level) {
  switch (level) {
    case constraint_level::none: return true;
    case constraint_level::nontrivial: return std::is_destructible_v<T>;
    case constraint_level::nothrow: return std::is_nothrow_destructible_v<T>;
    case constraint_level::trivial: return std::is_trivially_destructible_v<T>;
    default: return false;
  }
}

template <class T>
class destruction_guard {
 public:
  explicit destruction_guard(T* p) noexcept : p_(p) {}
  destruction_guard(const destruction_guard&) = delete;
  ~destruction_guard() noexcept(std::is_nothrow_destructible_v<T>)
      { std::destroy_at(p_); }

 private:
  T* p_;
};

template <bool IsDirect, class P, qualifier_type Q>
struct operand_traits : add_qualifier_traits<P, Q> {};
template <class P, qualifier_type Q>
struct operand_traits<false, P, Q>
    : std::type_identity<decltype(*std::declval<add_qualifier_t<P, Q>>())> {};
template <bool IsDirect, class P, qualifier_type Q>
using operand_t = typename operand_traits<IsDirect, P, Q>::type;

template <class D, bool NE, class R, class... Args>
concept invocable_dispatch = (NE && std::is_nothrow_invocable_r_v<
    R, D, Args...>) || (!NE && std::is_invocable_r_v<R, D, Args...>);
template <bool IsDirect, class D, class P, qualifier_type Q, bool NE, class R,
    class... Args>
concept invocable_dispatch_ptr =
    (IsDirect || (requires { *std::declval<add_qualifier_t<P, Q>>(); } &&
        (!NE || noexcept(*std::declval<add_qualifier_t<P, Q>>())))) &&
    invocable_dispatch<D, NE, R, operand_t<IsDirect, P, Q>, Args...> &&
    (Q != qualifier_type::rv || (NE && std::is_nothrow_destructible_v<P>) ||
        (!NE && std::is_destructible_v<P>));

template <class D, class R, class... Args>
R invoke_dispatch(Args&&... args) {
  if constexpr (std::is_void_v<R>) {
    D{}(std::forward<Args>(args)...);
  } else {
    return D{}(std::forward<Args>(args)...);
  }
}
template <bool IsDirect, class P, qualifier_type Q, class T>
decltype(auto) get_operand(T& ptr) {
  if constexpr (IsDirect) {
    return std::forward<add_qualifier_t<P, Q>>(ptr);
  } else {
    if constexpr (std::is_constructible_v<bool, T&>) { assert(ptr); }
    return *std::forward<add_qualifier_t<P, Q>>(ptr);
  }
}
template <bool IsDirect, class D, class P, qualifier_type Q, class R,
    class... Args>
R conv_dispatcher(add_qualifier_t<std::byte, Q> self, Args... args)
    noexcept(invocable_dispatch_ptr<IsDirect, D, P, Q, true, R, Args...>) {
  auto& qp = *std::launder(reinterpret_cast<add_qualifier_ptr_t<P, Q>>(&self));
  if constexpr (Q == qualifier_type::rv) {
    destruction_guard guard{&qp};
    return invoke_dispatch<D, R>(get_operand<IsDirect, P, Q>(qp),
        std::forward<Args>(args)...);
  } else {
    return invoke_dispatch<D, R>(get_operand<IsDirect, P, Q>(qp),
        std::forward<Args>(args)...);
  }
}
template <class D, qualifier_type Q, class R, class... Args>
R default_conv_dispatcher(add_qualifier_t<std::byte, Q>, Args... args)
    noexcept(invocable_dispatch<D, true, R, std::nullptr_t, Args...>)
    { return invoke_dispatch<D, R>(nullptr, std::forward<Args>(args)...); }

template <class O> struct overload_traits : inapplicable_traits {};
template <qualifier_type Q, bool NE, class R, class... Args>
struct overload_traits_impl : applicable_traits {
  using return_type = R;
  using view_type = R(Args...) const noexcept(NE);
  using dispatcher_type =
      R (*)(add_qualifier_t<std::byte, Q>, Args...) noexcept(NE);

  template <bool IsDirect, class D, class P>
  static consteval bool is_applicable_ptr() {
    if constexpr (invocable_dispatch_ptr<IsDirect, D, P, Q, NE, R, Args...>) {
      return true;
    } else {
      return invocable_dispatch<D, NE, R, std::nullptr_t, Args...>;
    }
  }
  template <bool IsDirect, class D, class P>
  static consteval dispatcher_type get_dispatcher() {
    if constexpr (invocable_dispatch_ptr<IsDirect, D, P, Q, NE, R, Args...>) {
      return &conv_dispatcher<IsDirect, D, P, Q, R, Args...>;
    } else {
      return &default_conv_dispatcher<D, Q, R, Args...>;
    }
  }

  static constexpr qualifier_type qualifier = Q;
};
template <class R, class... Args>
struct overload_traits<R(Args...)>
    : overload_traits_impl<qualifier_type::lv, false, R, Args...> {};
template <class R, class... Args>
struct overload_traits<R(Args...) noexcept>
    : overload_traits_impl<qualifier_type::lv, true, R, Args...> {};
template <class R, class... Args>
struct overload_traits<R(Args...) &>
    : overload_traits_impl<qualifier_type::lv, false, R, Args...> {};
template <class R, class... Args>
struct overload_traits<R(Args...) & noexcept>
    : overload_traits_impl<qualifier_type::lv, true, R, Args...> {};
template <class R, class... Args>
struct overload_traits<R(Args...) &&>
    : overload_traits_impl<qualifier_type::rv, false, R, Args...> {};
template <class R, class... Args>
struct overload_traits<R(Args...) && noexcept>
    : overload_traits_impl<qualifier_type::rv, true, R, Args...> {};
template <class R, class... Args>
struct overload_traits<R(Args...) const>
    : overload_traits_impl<qualifier_type::const_lv, false, R, Args...> {};
template <class R, class... Args>
struct overload_traits<R(Args...) const noexcept>
    : overload_traits_impl<qualifier_type::const_lv, true, R, Args...> {};
template <class R, class... Args>
struct overload_traits<R(Args...) const&>
    : overload_traits_impl<qualifier_type::const_lv, false, R, Args...> {};
template <class R, class... Args>
struct overload_traits<R(Args...) const& noexcept>
    : overload_traits_impl<qualifier_type::const_lv, true, R, Args...> {};
template <class R, class... Args>
struct overload_traits<R(Args...) const&&>
    : overload_traits_impl<qualifier_type::const_rv, false, R, Args...> {};
template <class R, class... Args>
struct overload_traits<R(Args...) const&& noexcept>
    : overload_traits_impl<qualifier_type::const_rv, true, R, Args...> {};

template <class O>
struct overload_substitution_traits
    : inapplicable_traits { template <class> using type = O; };
template <template <class> class O>
struct overload_substitution_traits<facade_aware_overload_t<O>>
    : applicable_traits { template <class F> using type = O<F>; };
template <class O, class F>
using substituted_overload_t =
    typename overload_substitution_traits<O>::template type<F>;
template <class O>
concept extended_overload = overload_traits<O>::applicable ||
    overload_substitution_traits<O>::applicable;
template <class P, class F, bool IsDirect, class D, class O>
consteval bool diagnose_proxiable_required_convention_not_implemented() {
  constexpr bool verdict =
      overload_traits<substituted_overload_t<O, F>>::applicable &&
      overload_traits<substituted_overload_t<O, F>>
          ::template is_applicable_ptr<IsDirect, D, P>();
  static_assert(verdict,
      "not proxiable due to a required convention not implemented");
  return verdict;
}

template <bool IsDirect, class D, class O>
struct invocation_meta {
  constexpr invocation_meta() noexcept : dispatcher(nullptr) {}
  template <class P>
  constexpr explicit invocation_meta(std::in_place_type_t<P>) noexcept
      : dispatcher(overload_traits<O>
            ::template get_dispatcher<IsDirect, D, P>()) {}

  typename overload_traits<O>::dispatcher_type dispatcher;
};

template <class... Ms>
struct composite_meta_impl : Ms... {
  constexpr composite_meta_impl() noexcept = default;
  template <class P>
  constexpr explicit composite_meta_impl(std::in_place_type_t<P>) noexcept
      : Ms(std::in_place_type<P>)... {}
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

template <class T>
consteval bool is_is_direct_well_formed() {
  if constexpr (requires { { T::is_direct } -> std::same_as<const bool&>; }) {
    if constexpr (is_consteval([] { return T::is_direct; })) {
      return true;
    }
  }
  return false;
}

template <class C, class... Os>
struct basic_conv_traits_impl : inapplicable_traits {};
template <class C, extended_overload... Os> requires(sizeof...(Os) > 0u)
struct basic_conv_traits_impl<C, Os...> : applicable_traits {};
template <class C> struct basic_conv_traits : inapplicable_traits {};
template <class C>
    requires(
        requires {
          typename C::dispatch_type;
          typename C::overload_types;
        } &&
        is_is_direct_well_formed<C>() &&
        std::is_trivial_v<typename C::dispatch_type> &&
        is_tuple_like_well_formed<typename C::overload_types>())
struct basic_conv_traits<C>
    : instantiated_t<basic_conv_traits_impl, typename C::overload_types, C> {};

template <class C, class F, class... Os>
struct conv_traits_impl : inapplicable_traits {};
template <class C, class F, class... Os>
    requires(overload_traits<substituted_overload_t<Os, F>>::applicable && ...)
struct conv_traits_impl<C, F, Os...> : applicable_traits {
  using meta = composite_meta_impl<invocation_meta<C::is_direct,
      typename C::dispatch_type, substituted_overload_t<Os, F>>...>;

  template <class P>
  static consteval bool diagnose_proxiable() {
    bool verdict = true;
    ((verdict &= diagnose_proxiable_required_convention_not_implemented<
        P, F, C::is_direct, typename C::dispatch_type, Os>()), ...);
    return verdict;
  }

  template <class P>
  static constexpr bool applicable_ptr = (overload_traits<
      substituted_overload_t<Os, F>>::template is_applicable_ptr<
      C::is_direct, typename C::dispatch_type, P>() && ...);
};
template <class C, class F>
struct conv_traits
    : instantiated_t<conv_traits_impl, typename C::overload_types, C, F> {};

template <class P>
using ptr_element_t = typename std::pointer_traits<P>::element_type;
template <bool IsDirect, class R>
struct refl_meta {
  template <class P> requires(IsDirect)
  constexpr explicit refl_meta(std::in_place_type_t<P>)
      : reflector(std::in_place_type<P>) {}
  template <class P> requires(!IsDirect)
  constexpr explicit refl_meta(std::in_place_type_t<P>)
      : reflector(std::in_place_type<ptr_element_t<P>>) {}

  R reflector;
};

template <class R> struct basic_refl_traits : inapplicable_traits {};
template <class R>
    requires(requires { typename R::reflector_type; } &&
        is_is_direct_well_formed<R>())
struct basic_refl_traits<R> : applicable_traits {};

template <class T, bool IsDirect, class R>
consteval bool is_reflector_well_formed() {
  if constexpr (IsDirect) {
    if constexpr (std::is_constructible_v<R, std::in_place_type_t<T>>) {
      if constexpr (is_consteval([] { return R{std::in_place_type<T>}; })) {
        return true;
      }
    }
  } else if constexpr (requires { typename ptr_element_t<T>; }) {
    return is_reflector_well_formed<ptr_element_t<T>, true, R>();
  }
  return false;
}
template <class P, class F, bool IsDirect, class R>
consteval bool diagnose_proxiable_required_reflection_not_implemented() {
  constexpr bool verdict = is_reflector_well_formed<P, IsDirect, R>();
  static_assert(verdict,
      "not proxiable due to a required reflection not implemented");
  return verdict;
}

template <class R>
struct refl_traits {
  using meta = refl_meta<R::is_direct, typename R::reflector_type>;

  template <class P, class F>
  static consteval bool diagnose_proxiable() {
    return diagnose_proxiable_required_reflection_not_implemented<
        P, F, R::is_direct, typename R::reflector_type>();
  }

  template <class P>
  static constexpr bool applicable_ptr =
      is_reflector_well_formed<P, R::is_direct, typename R::reflector_type>();
};

struct copy_dispatch {
  template <class T, class F>
  ___PRO3_STATIC_CALL(void, T&& self, proxy<F>& rhs)
      noexcept(std::is_nothrow_constructible_v<std::decay_t<T>, T>)
      requires(std::is_constructible_v<std::decay_t<T>, T>)
      { std::construct_at(&rhs, std::forward<T>(self)); }
};
struct destroy_dispatch {
  template <class T>
  ___PRO3_STATIC_CALL(void, T& self) noexcept(std::is_nothrow_destructible_v<T>)
      requires(std::is_destructible_v<T>) { std::destroy_at(&self); }
};
template <class D, class ONE, class OE, constraint_level C>
struct lifetime_meta_traits : std::type_identity<void> {};
template <class D, class ONE, class OE>
struct lifetime_meta_traits<D, ONE, OE, constraint_level::nothrow>
    : std::type_identity<invocation_meta<true, D, ONE>> {};
template <class D, class ONE, class OE>
struct lifetime_meta_traits<D, ONE, OE, constraint_level::nontrivial>
    : std::type_identity<invocation_meta<true, D, OE>> {};
template <class D, class ONE, class OE, constraint_level C>
using lifetime_meta_t = typename lifetime_meta_traits<D, ONE, OE, C>::type;

template <class... As>
class ___PRO3_ENFORCE_EBO composite_accessor_impl : public As... {
  template <facade> friend class pro::v3::proxy;
  template <facade> friend struct pro::v3::proxy_indirect_accessor;

  composite_accessor_impl() noexcept = default;
  composite_accessor_impl(const composite_accessor_impl&) noexcept = default;
  composite_accessor_impl& operator=(const composite_accessor_impl&) noexcept
      = default;
};

template <class T>
struct accessor_traits_impl : std::type_identity<void> {};
template <class T>
    requires(std::is_nothrow_default_constructible_v<T> &&
        std::is_trivially_copyable_v<T> && !std::is_final_v<T>)
struct accessor_traits_impl<T> : std::type_identity<T> {};
template <class SFINAE, class T, class F>
struct accessor_traits : std::type_identity<void> {};
template <class T, class F>
struct accessor_traits<std::void_t<typename T::template accessor<F>>, T, F>
    : accessor_traits_impl<typename T::template accessor<F>> {};
template <class T, class F>
using accessor_t = typename accessor_traits<void, T, F>::type;

template <bool IsDirect, class F, class O, class I>
struct composite_accessor_reduction : std::type_identity<O> {};
template <bool IsDirect, class F, class... As, class I>
    requires(IsDirect == I::is_direct && !std::is_void_v<accessor_t<I, F>>)
struct composite_accessor_reduction<
    IsDirect, F, composite_accessor_impl<As...>, I>
    : std::type_identity<composite_accessor_impl<As..., accessor_t<I, F>>> {};
template <bool IsDirect, class F>
struct composite_accessor_helper {
  template <class O, class I>
  using reduction_t =
      typename composite_accessor_reduction<IsDirect, F, O, I>::type;
};
template <bool IsDirect, class F, class... Ts>
using composite_accessor = recursive_reduction_t<
      composite_accessor_helper<IsDirect, F>::template reduction_t,
      composite_accessor_impl<>, Ts...>;

template <class A1, class A2> struct composite_accessor_merge_traits;
template <class... A1, class... A2>
struct composite_accessor_merge_traits<
    composite_accessor_impl<A1...>, composite_accessor_impl<A2...>>
    : std::type_identity<composite_accessor_impl<A1..., A2...>> {};
template <class A1, class A2>
using merged_composite_accessor =
    typename composite_accessor_merge_traits<A1, A2>::type;

template <class T> struct in_place_type_traits : inapplicable_traits {};
template <class T>
struct in_place_type_traits<std::in_place_type_t<T>> : applicable_traits {};

template <class P> struct proxy_traits : inapplicable_traits {};
template <class F>
struct proxy_traits<proxy<F>> : applicable_traits { using facade_type = F; };

template <class P, class F, std::size_t ActualSize, std::size_t MaxSize>
consteval bool diagnose_proxiable_size_too_large() {
  constexpr bool verdict = ActualSize <= MaxSize;
  static_assert(verdict, "not proxiable due to size too large");
  return verdict;
}
template <class P, class F, std::size_t ActualAlign, std::size_t MaxAlign>
consteval bool diagnose_proxiable_align_too_large() {
  constexpr bool verdict = ActualAlign <= MaxAlign;
  static_assert(verdict, "not proxiable due to alignment too large");
  return verdict;
}
template <class P, class F, constraint_level RequiredCopyability>
consteval bool diagnose_proxiable_insufficient_copyability() {
  constexpr bool verdict = has_copyability<P>(RequiredCopyability);
  static_assert(verdict, "not proxiable due to insufficient copyability");
  return verdict;
}
template <class P, class F, constraint_level RequiredRelocatability>
consteval bool diagnose_proxiable_insufficient_relocatability() {
  constexpr bool verdict = has_relocatability<P>(RequiredRelocatability);
  static_assert(verdict, "not proxiable due to insufficient relocatability");
  return verdict;
}
template <class P, class F, constraint_level RequiredDestructibility>
consteval bool diagnose_proxiable_insufficient_destructibility() {
  constexpr bool verdict = has_destructibility<P>(RequiredDestructibility);
  static_assert(verdict, "not proxiable due to insufficient destructibility");
  return verdict;
}

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
template <class... Cs>
struct basic_facade_conv_traits_impl : inapplicable_traits {};
template <class... Cs> requires(basic_conv_traits<Cs>::applicable && ...)
struct basic_facade_conv_traits_impl<Cs...> : applicable_traits {};
template <class... Rs>
struct basic_facade_refl_traits_impl : inapplicable_traits {};
template <class... Rs> requires(basic_refl_traits<Rs>::applicable && ...)
struct basic_facade_refl_traits_impl<Rs...> : applicable_traits {};
template <class F> struct basic_facade_traits : inapplicable_traits {};
template <class F>
    requires(
        requires {
          typename F::convention_types;
          typename F::reflection_types;
        } &&
        is_facade_constraints_well_formed<F>() &&
        is_tuple_like_well_formed<typename F::convention_types>() &&
        instantiated_t<basic_facade_conv_traits_impl,
            typename F::convention_types>::applicable &&
        is_tuple_like_well_formed<typename F::reflection_types>() &&
        instantiated_t<basic_facade_refl_traits_impl,
            typename F::reflection_types>::applicable)
struct basic_facade_traits<F> : applicable_traits {};

template <class F, class... Cs>
struct facade_conv_traits_impl : inapplicable_traits {};
template <class F, class... Cs> requires(conv_traits<Cs, F>::applicable && ...)
struct facade_conv_traits_impl<F, Cs...> : applicable_traits {
  using conv_meta = composite_meta<typename conv_traits<Cs, F>::meta...>;
  using conv_indirect_accessor = composite_accessor<false, F, Cs...>;
  using conv_direct_accessor = composite_accessor<true, F, Cs...>;

  template <class P>
  static consteval bool diagnose_proxiable_conv() {
    bool verdict = true;
    ((verdict &= conv_traits<Cs, F>::template diagnose_proxiable<P>()), ...);
    return verdict;
  }

  template <class P>
  static constexpr bool conv_applicable_ptr =
      (conv_traits<Cs, F>::template applicable_ptr<P> && ...);
  template <bool IsDirect, class D, class O>
  static constexpr bool is_invocable = std::is_base_of_v<
      invocation_meta<IsDirect, D, O>, conv_meta>;
};
template <class F, class... Rs>
struct facade_refl_traits_impl {
  using refl_meta = composite_meta<typename refl_traits<Rs>::meta...>;
  using refl_indirect_accessor = composite_accessor<false, F, Rs...>;
  using refl_direct_accessor = composite_accessor<true, F, Rs...>;

  template <class P>
  static consteval bool diagnose_proxiable_refl() {
    bool verdict = true;
    ((verdict &= refl_traits<Rs>::template diagnose_proxiable<P, F>()), ...);
    return verdict;
  }

  template <class P>
  static constexpr bool refl_applicable_ptr =
      (refl_traits<Rs>::template applicable_ptr<P> && ...);
};
template <class F> struct facade_traits : inapplicable_traits {};
template <class F> requires(instantiated_t<
    facade_conv_traits_impl, typename F::convention_types, F>::applicable)
struct facade_traits<F>
    : instantiated_t<facade_conv_traits_impl, typename F::convention_types, F>,
      instantiated_t<facade_refl_traits_impl, typename F::reflection_types, F> {
  using meta = composite_meta<
      lifetime_meta_t<copy_dispatch, void(proxy<F>&) const noexcept,
          void(proxy<F>&) const, F::constraints.copyability>,
      lifetime_meta_t<copy_dispatch, void(proxy<F>&) && noexcept,
          void(proxy<F>&) &&, F::constraints.relocatability>,
      lifetime_meta_t<destroy_dispatch, void() noexcept, void(),
          F::constraints.destructibility>, typename facade_traits::conv_meta,
      typename facade_traits::refl_meta>;
  using indirect_accessor = merged_composite_accessor<
      typename facade_traits::conv_indirect_accessor,
      typename facade_traits::refl_indirect_accessor>;
  using direct_accessor = merged_composite_accessor<
      typename facade_traits::conv_direct_accessor,
      typename facade_traits::refl_direct_accessor>;

  template <class P>
  static consteval void diagnose_proxiable() {
    bool verdict = true;
    verdict &= diagnose_proxiable_size_too_large<
        P, F, sizeof(P), F::constraints.max_size>();
    verdict &= diagnose_proxiable_align_too_large<
        P, F, alignof(P), F::constraints.max_align>();
    verdict &= diagnose_proxiable_insufficient_copyability<
        P, F, F::constraints.copyability>();
    verdict &= diagnose_proxiable_insufficient_relocatability<
        P, F, F::constraints.relocatability>();
    verdict &= diagnose_proxiable_insufficient_destructibility<
        P, F, F::constraints.destructibility>();
    verdict &= facade_traits::template diagnose_proxiable_conv<P>();
    verdict &= facade_traits::template diagnose_proxiable_refl<P>();
    if (!verdict) { std::abort(); }  // Propagate the error to the caller side
  }

  template <class P>
  static constexpr bool applicable_ptr = sizeof(P) <= F::constraints.max_size &&
      alignof(P) <= F::constraints.max_align &&
      has_copyability<P>(F::constraints.copyability) &&
      has_relocatability<P>(F::constraints.relocatability) &&
      has_destructibility<P>(F::constraints.destructibility) &&
      facade_traits::template conv_applicable_ptr<P> &&
      facade_traits::template refl_applicable_ptr<P>;
};

using ptr_prototype = void*[2];

template <class M>
struct meta_ptr_indirect_impl {
  constexpr meta_ptr_indirect_impl() noexcept : ptr_(nullptr) {};
  template <class P>
  constexpr explicit meta_ptr_indirect_impl(std::in_place_type_t<P>) noexcept
      : ptr_(&storage<P>) {}
  bool has_value() const noexcept { return ptr_ != nullptr; }
  void reset() noexcept { ptr_ = nullptr; }
  const M* operator->() const noexcept { return ptr_; }

 private:
  const M* ptr_;
  template <class P> static constexpr M storage{std::in_place_type<P>};
};
template <class M, class DM>
struct meta_ptr_direct_impl : private M {
  using M::M;
  bool has_value() const noexcept { return this->DM::dispatcher != nullptr; }
  void reset() noexcept { this->DM::dispatcher = nullptr; }
  const M* operator->() const noexcept { return this; }
};
template <class M>
struct meta_ptr_traits_impl : std::type_identity<meta_ptr_indirect_impl<M>> {};
template <bool IsDirect, class D, class O, class... Ms>
struct meta_ptr_traits_impl<
    composite_meta_impl<invocation_meta<IsDirect, D, O>, Ms...>>
    : std::type_identity<meta_ptr_direct_impl<composite_meta_impl<
          invocation_meta<IsDirect, D, O>, Ms...>,
          invocation_meta<IsDirect, D, O>>> {};
template <class M>
struct meta_ptr_traits : std::type_identity<meta_ptr_indirect_impl<M>> {};
template <class M>
    requires(sizeof(M) <= sizeof(ptr_prototype) &&
        alignof(M) <= alignof(ptr_prototype) &&
        std::is_nothrow_default_constructible_v<M> &&
        std::is_trivially_copyable_v<M>)
struct meta_ptr_traits<M> : meta_ptr_traits_impl<M> {};
template <class M>
using meta_ptr = typename meta_ptr_traits<M>::type;

template <class MP>
struct meta_ptr_reset_guard {
 public:
  explicit meta_ptr_reset_guard(MP& meta) noexcept : meta_(meta) {}
  meta_ptr_reset_guard(const meta_ptr_reset_guard&) = delete;
  ~meta_ptr_reset_guard() { meta_.reset(); }

 private:
  MP& meta_;
};

template <class F>
struct proxy_helper {
  static inline const auto& get_meta(const proxy<F>& p) noexcept {
    assert(p.has_value());
    return *p.meta_.operator->();
  }
  template <bool IsDirect, class D, class O, qualifier_type Q, class... Args>
  static decltype(auto) invoke(add_qualifier_t<proxy<F>, Q> p, Args&&... args) {
    auto dispatcher = get_meta(p).template invocation_meta<IsDirect, D, O>
        ::dispatcher;
    if constexpr (overload_traits<O>::qualifier == qualifier_type::rv) {
      meta_ptr_reset_guard guard{p.meta_};
      return dispatcher(std::forward<add_qualifier_t<std::byte, Q>>(*p.ptr_),
          std::forward<Args>(args)...);
    } else {
      return dispatcher(std::forward<add_qualifier_t<std::byte, Q>>(*p.ptr_),
          std::forward<Args>(args)...);
    }
  }
  template <class A, qualifier_type Q>
  static add_qualifier_t<proxy<F>, Q> access(add_qualifier_t<A, Q> a) {
    if constexpr (std::is_base_of_v<A, proxy<F>>) {
      return static_cast<add_qualifier_t<proxy<F>, Q>>(
          std::forward<add_qualifier_t<A, Q>>(a));
    } else {
      // Note: The use of offsetof below is technically undefined until C++20
      // because proxy may not be a standard layout type. However, all compilers
      // currently provide well-defined behavior as an extension (which is
      // demonstrated since constexpr evaluation must diagnose all undefined
      // behavior). However, various compilers also warn about this use of
      // offsetof, which must be suppressed.
#if defined(__INTEL_COMPILER)
#pragma warning push
#pragma warning(disable : 1875)
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif  // defined(__INTEL_COMPILER)
#if defined(__NVCC__)
#pragma nv_diagnostic push
#pragma nv_diag_suppress 1427
#endif  // defined(__NVCC__)
#if defined(__NVCOMPILER)
#pragma diagnostic push
#pragma diag_suppress offset_in_non_POD_nonstandard
#endif  // defined(__NVCOMPILER)
      constexpr std::size_t offset = offsetof(proxy<F>, value_);
#if defined(__INTEL_COMPILER)
#pragma warning pop
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif  // defined(__INTEL_COMPILER)
#if defined(__NVCC__)
#pragma nv_diagnostic pop
#endif  // defined(__NVCC__)
#if defined(__NVCOMPILER)
#pragma diagnostic pop
#endif  // defined(__NVCOMPILER)
      return reinterpret_cast<add_qualifier_t<proxy<F>, Q>>(*(reinterpret_cast<
          add_qualifier_ptr_t<std::byte, Q>>(static_cast<add_qualifier_ptr_t<
              proxy_indirect_accessor<F>, Q>>(std::addressof(a))) - offset));
    }
  }
};

template <class T>
class inplace_ptr {
  template <class> friend struct proxy_helper;

 public:
  template <class... Args>
  explicit inplace_ptr(std::in_place_t, Args&&... args)
      : value_(std::forward<Args>(args)...) {}
  inplace_ptr() = default;
  inplace_ptr(const inplace_ptr&) = default;
  inplace_ptr(inplace_ptr&&) = default;
  inplace_ptr& operator=(const inplace_ptr&) = default;
  inplace_ptr& operator=(inplace_ptr&&) = default;

  T* operator->() noexcept { return std::addressof(value_); }
  const T* operator->() const noexcept { return std::addressof(value_); }
  T& operator*() & noexcept { return value_; }
  const T& operator*() const& noexcept { return value_; }
  T&& operator*() && noexcept { return std::move(value_); }
  const T&& operator*() const&& noexcept { return std::move(value_); }

 private:
  [[___PRO_NO_UNIQUE_ADDRESS_ATTRIBUTE]]
  T value_;
};

}  // namespace details

template <class P, class F>
concept proxiable = facade<F> && details::facade_traits<F>::applicable &&
    details::facade_traits<F>::template applicable_ptr<P>;

template <facade F>
struct proxy_indirect_accessor : details::facade_traits<F>::indirect_accessor
    { friend class details::inplace_ptr<proxy_indirect_accessor>; };

template <bool IsDirect, class D, class O, facade F, class... Args>
auto proxy_invoke(proxy<F>& p, Args&&... args)
    -> typename details::overload_traits<O>::return_type {
  return details::proxy_helper<F>::template invoke<IsDirect, D, O,
      details::qualifier_type::lv>(p, std::forward<Args>(args)...);
}
template <bool IsDirect, class D, class O, facade F, class... Args>
auto proxy_invoke(const proxy<F>& p, Args&&... args)
    -> typename details::overload_traits<O>::return_type {
  return details::proxy_helper<F>::template invoke<IsDirect, D, O,
      details::qualifier_type::const_lv>(p, std::forward<Args>(args)...);
}
template <bool IsDirect, class D, class O, facade F, class... Args>
auto proxy_invoke(proxy<F>&& p, Args&&... args)
    -> typename details::overload_traits<O>::return_type {
  return details::proxy_helper<F>::template invoke<
      IsDirect, D, O, details::qualifier_type::rv>(
      std::move(p), std::forward<Args>(args)...);
}
template <bool IsDirect, class D, class O, facade F, class... Args>
auto proxy_invoke(const proxy<F>&& p, Args&&... args)
    -> typename details::overload_traits<O>::return_type {
  return details::proxy_helper<F>::template invoke<
      IsDirect, D, O, details::qualifier_type::const_rv>(
      std::move(p), std::forward<Args>(args)...);
}

template <bool IsDirect, class R, facade F>
const R& proxy_reflect(const proxy<F>& p) noexcept {
  return static_cast<const details::refl_meta<IsDirect, R>&>(
      details::proxy_helper<F>::get_meta(p)).reflector;
}

template <facade F, class A>
proxy<F>& access_proxy(A& a) noexcept {
  return details::proxy_helper<F>::template access<
      A, details::qualifier_type::lv>(a);
}
template <facade F, class A>
const proxy<F>& access_proxy(const A& a) noexcept {
  return details::proxy_helper<F>::template access<
      A, details::qualifier_type::const_lv>(a);
}
template <facade F, class A>
proxy<F>&& access_proxy(A&& a) noexcept {
  return details::proxy_helper<F>::template access<
      A, details::qualifier_type::rv>(std::forward<A>(a));
}
template <facade F, class A>
const proxy<F>&& access_proxy(const A&& a) noexcept {
  return details::proxy_helper<F>::template access<
      A, details::qualifier_type::const_rv>(std::forward<const A>(a));
}

template <facade F>
class proxy : public details::facade_traits<F>::direct_accessor,
    public details::inplace_ptr<proxy_indirect_accessor<F>> {
  friend struct details::proxy_helper<F>;
  using _Traits = details::facade_traits<F>;
  static_assert(_Traits::applicable);

 public:
  proxy() noexcept { ___PRO3_DEBUG(std::ignore = &_symbol_guard;) }
  proxy(std::nullptr_t) noexcept : proxy() {}
  proxy(const proxy&) noexcept requires(F::constraints.copyability ==
      constraint_level::trivial) = default;
  proxy(const proxy& rhs)
      noexcept(F::constraints.copyability == constraint_level::nothrow)
      requires(F::constraints.copyability == constraint_level::nontrivial ||
          F::constraints.copyability == constraint_level::nothrow)
      : details::inplace_ptr<proxy_indirect_accessor<F>>() {  // Make GCC happy
    if (rhs.meta_.has_value()) {
      proxy_invoke<true, details::copy_dispatch, void(proxy&) const noexcept(
          F::constraints.copyability == constraint_level::nothrow)>(rhs, *this);
    }
  }
  proxy(proxy&& rhs)
      noexcept(F::constraints.relocatability == constraint_level::nothrow)
      requires(F::constraints.relocatability >= constraint_level::nontrivial &&
          F::constraints.copyability != constraint_level::trivial) {
    if (rhs.meta_.has_value()) {
      details::meta_ptr_reset_guard guard{rhs.meta_};
      if constexpr (F::constraints.relocatability ==
          constraint_level::trivial) {
        std::ranges::uninitialized_copy(rhs.ptr_, ptr_);
        meta_ = rhs.meta_;
      } else {
        proxy_invoke<true, details::copy_dispatch, void(proxy&) && noexcept(
            F::constraints.relocatability == constraint_level::nothrow)>(
            std::move(rhs), *this);
      }
    }
  }
  template <class P>
  constexpr proxy(P&& ptr)
      noexcept(std::is_nothrow_constructible_v<std::decay_t<P>, P>)
      requires(!details::proxy_traits<std::decay_t<P>>::applicable &&
          !details::in_place_type_traits<std::decay_t<P>>::applicable &&
          std::is_constructible_v<std::decay_t<P>, P>)
      : proxy() { initialize<std::decay_t<P>>(std::forward<P>(ptr)); }
  template <class P, class... Args>
  constexpr explicit proxy(std::in_place_type_t<P>, Args&&... args)
      noexcept(std::is_nothrow_constructible_v<P, Args...>)
      requires(std::is_constructible_v<P, Args...>)
      : proxy() { initialize<P>(std::forward<Args>(args)...); }
  template <class P, class U, class... Args>
  constexpr explicit proxy(std::in_place_type_t<P>, std::initializer_list<U> il,
          Args&&... args)
      noexcept(std::is_nothrow_constructible_v<
          P, std::initializer_list<U>&, Args...>)
      requires(std::is_constructible_v<P, std::initializer_list<U>&, Args...>)
      : proxy() { initialize<P>(il, std::forward<Args>(args)...); }
  proxy& operator=(std::nullptr_t)
      noexcept(F::constraints.destructibility >= constraint_level::nothrow)
      requires(F::constraints.destructibility >= constraint_level::nontrivial)
      { reset(); return *this; }
  proxy& operator=(const proxy&) noexcept requires(F::constraints.copyability ==
      constraint_level::trivial) = default;
  proxy& operator=(const proxy& rhs)
      noexcept(F::constraints.copyability >= constraint_level::nothrow &&
          F::constraints.destructibility >= constraint_level::nothrow)
      requires((F::constraints.copyability == constraint_level::nontrivial ||
          F::constraints.copyability == constraint_level::nothrow) &&
          F::constraints.destructibility >= constraint_level::nontrivial) {
    if (this != &rhs) {
      if constexpr (F::constraints.copyability == constraint_level::nothrow) {
        std::destroy_at(this);
        std::construct_at(this, rhs);
      } else {
        *this = proxy{rhs};
      }
    }
    return *this;
  }
  proxy& operator=(proxy&& rhs)
      noexcept(F::constraints.relocatability >= constraint_level::nothrow &&
          F::constraints.destructibility >= constraint_level::nothrow)
      requires(F::constraints.relocatability >= constraint_level::nontrivial &&
          F::constraints.destructibility >= constraint_level::nontrivial &&
          F::constraints.copyability != constraint_level::trivial) {
    if (this != &rhs) {
      reset();
      std::construct_at(this, std::move(rhs));
    }
    return *this;
  }
  template <class P>
  constexpr proxy& operator=(P&& ptr)
      noexcept(std::is_nothrow_constructible_v<std::decay_t<P>, P> &&
          F::constraints.destructibility >= constraint_level::nothrow)
      requires(!details::proxy_traits<std::decay_t<P>>::applicable &&
          std::is_constructible_v<std::decay_t<P>, P> &&
          F::constraints.destructibility >= constraint_level::nontrivial) {
    if constexpr (std::is_nothrow_constructible_v<std::decay_t<P>, P>) {
      std::destroy_at(this);
      initialize<std::decay_t<P>>(std::forward<P>(ptr));
    } else {
      *this = proxy{std::forward<P>(ptr)};
    }
    return *this;
  }
  ~proxy() requires(F::constraints.destructibility == constraint_level::trivial)
      = default;
  ~proxy() noexcept(F::constraints.destructibility == constraint_level::nothrow)
      requires(F::constraints.destructibility == constraint_level::nontrivial ||
          F::constraints.destructibility == constraint_level::nothrow) {
    if (meta_.has_value()) {
      proxy_invoke<true, details::destroy_dispatch, void() noexcept(
          F::constraints.destructibility == constraint_level::nothrow)>(*this);
    }
  }

  bool has_value() const noexcept { return meta_.has_value(); }
  explicit operator bool() const noexcept { return meta_.has_value(); }
  void reset()
      noexcept(F::constraints.destructibility >= constraint_level::nothrow)
      requires(F::constraints.destructibility >= constraint_level::nontrivial)
      { std::destroy_at(this); meta_.reset(); }
  void swap(proxy& rhs)
      noexcept(F::constraints.relocatability >= constraint_level::nothrow ||
          F::constraints.copyability == constraint_level::trivial)
      requires(F::constraints.relocatability >= constraint_level::nontrivial ||
          F::constraints.copyability == constraint_level::trivial) {
    if constexpr (F::constraints.relocatability == constraint_level::trivial ||
        F::constraints.copyability == constraint_level::trivial) {
      std::swap(meta_, rhs.meta_);
      std::swap(ptr_, rhs.ptr_);
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
  template <class P, class... Args>
  constexpr P& emplace(Args&&... args)
      noexcept(std::is_nothrow_constructible_v<P, Args...> &&
          F::constraints.destructibility >= constraint_level::nothrow)
      requires(std::is_constructible_v<P, Args...> &&
          F::constraints.destructibility >= constraint_level::nontrivial)
      { reset(); return initialize<P>(std::forward<Args>(args)...); }
  template <class P, class U, class... Args>
  constexpr P& emplace(std::initializer_list<U> il, Args&&... args)
      noexcept(std::is_nothrow_constructible_v<
          P, std::initializer_list<U>&, Args...> &&
          F::constraints.destructibility >= constraint_level::nothrow)
      requires(std::is_constructible_v<P, std::initializer_list<U>&, Args...> &&
          F::constraints.destructibility >= constraint_level::nontrivial)
      { reset(); return initialize<P>(il, std::forward<Args>(args)...); }

  friend void swap(proxy& lhs, proxy& rhs) noexcept(noexcept(lhs.swap(rhs)))
      { lhs.swap(rhs); }
  friend bool operator==(const proxy& lhs, std::nullptr_t) noexcept
      { return !lhs.has_value(); }

 private:
  template <class P, class... Args>
  constexpr P& initialize(Args&&... args) {
    P& result = *std::construct_at(
        reinterpret_cast<P*>(ptr_), std::forward<Args>(args)...);
    if constexpr (proxiable<P, F>) {
      meta_ = details::meta_ptr<typename _Traits::meta>{std::in_place_type<P>};
    } else {
      _Traits::template diagnose_proxiable<P>();
    }
    return result;
  }

___PRO3_DEBUG(
  static inline void _symbol_guard(proxy& self, const proxy& cself) noexcept {
    self.operator->(); *self; *std::move(self);
    cself.operator->(); *cself; *std::move(cself);
  }
)

  details::meta_ptr<typename _Traits::meta> meta_;
  alignas(F::constraints.max_align) std::byte ptr_[F::constraints.max_size];
};

namespace details {

template <class LR, class CLR, class RR, class CRR>
struct observer_ptr {
 public:
  explicit observer_ptr(LR lr) : lr_(lr) {}
  observer_ptr(const observer_ptr&) = default;
  auto operator->() noexcept { return std::addressof(lr_); }
  auto operator->() const noexcept
      { return std::addressof(static_cast<CLR>(lr_)); }
  LR operator*() & noexcept { return static_cast<LR>(lr_); }
  CLR operator*() const& noexcept { return static_cast<CLR>(lr_); }
  RR operator*() && noexcept { return static_cast<RR>(lr_); }
  CRR operator*() const&& noexcept { return static_cast<CRR>(lr_); }

 private:
  LR lr_;
};

#if __STDC_HOSTED__
template <class T, class Alloc, class... Args>
T* allocate(const Alloc& alloc, Args&&... args) {
  auto al = typename std::allocator_traits<Alloc>
      ::template rebind_alloc<T>(alloc);
  auto deleter = [&](T* ptr) { al.deallocate(ptr, 1); };
  std::unique_ptr<T, decltype(deleter)> result{al.allocate(1), deleter};
  std::construct_at(result.get(), std::forward<Args>(args)...);
  return result.release();
}
template <class Alloc, class T>
void deallocate(const Alloc& alloc, T* ptr) {
  auto al = typename std::allocator_traits<Alloc>
      ::template rebind_alloc<T>(alloc);
  std::destroy_at(ptr);
  al.deallocate(ptr, 1);
}
template <class Alloc>
struct alloc_aware {
 public:
  explicit alloc_aware(const Alloc& alloc) noexcept : alloc(alloc) {}
  alloc_aware(const alloc_aware&) noexcept = default;

  [[___PRO_NO_UNIQUE_ADDRESS_ATTRIBUTE]]
  Alloc alloc;
};
template <class T>
class indirect_ptr {
 public:
  explicit indirect_ptr(T* ptr) noexcept : ptr_(ptr) {}

  explicit operator bool() const noexcept { return ptr_ != nullptr; }
  auto operator->() noexcept { return std::addressof(**ptr_); }
  auto operator->() const noexcept { return std::addressof(**ptr_); }
  decltype(auto) operator*() & noexcept { return **ptr_; }
  decltype(auto) operator*() const& noexcept { return *std::as_const(*ptr_); }
  decltype(auto) operator*() && noexcept { return *std::move(*ptr_); }
  decltype(auto) operator*() const&& noexcept
      { return *std::move(std::as_const(*ptr_)); }

 protected:
  T* ptr_;
};

template <class T, class Alloc>
class ___PRO3_ENFORCE_EBO allocated_ptr
    : private alloc_aware<Alloc>, public indirect_ptr<inplace_ptr<T>> {
 public:
  template <class... Args>
  allocated_ptr(const Alloc& alloc, Args&&... args)
      : alloc_aware<Alloc>(alloc),
        indirect_ptr<inplace_ptr<T>>(allocate<inplace_ptr<T>>(
            this->alloc, std::in_place, std::forward<Args>(args)...)) {}
  allocated_ptr(const allocated_ptr& rhs)
      requires(std::is_copy_constructible_v<T>)
      : alloc_aware<Alloc>(rhs), indirect_ptr<inplace_ptr<T>>(
            rhs.ptr_ == nullptr ? nullptr : allocate<inplace_ptr<T>>(
                this->alloc, std::in_place, *rhs)) {}
  allocated_ptr(allocated_ptr&& rhs)
      noexcept(std::is_nothrow_move_constructible_v<Alloc>)
      : alloc_aware<Alloc>(rhs),
        indirect_ptr<inplace_ptr<T>>(std::exchange(rhs.ptr_, nullptr)) {}
  ~allocated_ptr() noexcept(std::is_nothrow_destructible_v<T>)
      { if (this->ptr_ != nullptr) { deallocate(this->alloc, this->ptr_); } }
};

template <class T, class Alloc>
struct ___PRO3_ENFORCE_EBO compact_ptr_storage
    : alloc_aware<Alloc>, inplace_ptr<T> {
  template <class... Args>
  explicit compact_ptr_storage(const Alloc& alloc, Args&&... args)
      : alloc_aware<Alloc>(alloc),
        inplace_ptr<T>(std::in_place, std::forward<Args>(args)...) {}
};
template <class T, class Alloc>
class compact_ptr : public indirect_ptr<compact_ptr_storage<T, Alloc>> {
  using Storage = compact_ptr_storage<T, Alloc>;

 public:
  template <class... Args>
  compact_ptr(const Alloc& alloc, Args&&... args)
      : indirect_ptr<Storage>(allocate<Storage>(
            alloc, alloc, std::forward<Args>(args)...)) {}
  compact_ptr(const compact_ptr& rhs) requires(std::is_copy_constructible_v<T>)
      : indirect_ptr<Storage>(rhs.ptr_ == nullptr ? nullptr :
            allocate<Storage>(rhs.ptr_->alloc, rhs.ptr_->alloc, *rhs)) {}
  compact_ptr(compact_ptr&& rhs) noexcept
      : indirect_ptr<Storage>(std::exchange(rhs.ptr_, nullptr)) {}
  ~compact_ptr() noexcept(std::is_nothrow_destructible_v<T>) {
    if (this->ptr_ != nullptr) { deallocate(this->ptr_->alloc, this->ptr_); }
  }
};

struct shared_compact_ptr_storage_base { std::atomic_long ref_count = 1; };
template <class T, class Alloc>
struct ___PRO3_ENFORCE_EBO shared_compact_ptr_storage
    : shared_compact_ptr_storage_base, alloc_aware<Alloc>, inplace_ptr<T> {
  template <class... Args>
  explicit shared_compact_ptr_storage(const Alloc& alloc, Args&&... args)
      : alloc_aware<Alloc>(alloc),
        inplace_ptr<T>(std::in_place, std::forward<Args>(args)...) {}
};
template <class T, class Alloc>
class shared_compact_ptr
    : public indirect_ptr<shared_compact_ptr_storage<T, Alloc>> {
  using Storage = shared_compact_ptr_storage<T, Alloc>;

 public:
  template <class... Args>
  shared_compact_ptr(const Alloc& alloc, Args&&... args)
      : indirect_ptr<Storage>(allocate<Storage>(
            alloc, alloc, std::forward<Args>(args)...)) {}
  shared_compact_ptr(const shared_compact_ptr& rhs) noexcept
      : indirect_ptr<Storage>(rhs.ptr_) {
    if (this->ptr_ != nullptr)
        { this->ptr_->ref_count.fetch_add(1, std::memory_order::relaxed); }
  }
  shared_compact_ptr(shared_compact_ptr&& rhs) noexcept
      : indirect_ptr<Storage>(std::exchange(rhs.ptr_, nullptr)) {}
  ~shared_compact_ptr() noexcept(std::is_nothrow_destructible_v<T>) {
    if (this->ptr_ != nullptr &&
        this->ptr_->ref_count.fetch_sub(1, std::memory_order::acq_rel) == 1) {
      deallocate(this->ptr_->alloc, this->ptr_);
    }
  }
};

struct strong_weak_compact_ptr_storage_base
    { std::atomic_long strong_count = 1, weak_count = 1; };
template <class T, class Alloc>
struct strong_weak_compact_ptr_storage
    : strong_weak_compact_ptr_storage_base, alloc_aware<Alloc> {
  template <class... Args>
  explicit strong_weak_compact_ptr_storage(const Alloc& alloc, Args&&... args)
      : alloc_aware<Alloc>(alloc) {
    std::construct_at(
        reinterpret_cast<T*>(&value), std::forward<Args>(args)...);
  }

  alignas(alignof(T)) std::byte value[sizeof(T)];
};
template <class T, class Alloc> class weak_compact_ptr;
template <class T, class Alloc>
class strong_compact_ptr
    : public indirect_ptr<strong_weak_compact_ptr_storage<T, Alloc>> {
  friend class weak_compact_ptr<T, Alloc>;
  using Storage = strong_weak_compact_ptr_storage<T, Alloc>;

 public:
  using weak_type = weak_compact_ptr<T, Alloc>;

  template <class... Args>
  strong_compact_ptr(const Alloc& alloc, Args&&... args)
      : indirect_ptr<Storage>(allocate<Storage>(
            alloc, alloc, std::forward<Args>(args)...)) {}
  strong_compact_ptr(const strong_compact_ptr& rhs) noexcept
      : indirect_ptr<Storage>(rhs.ptr_) {
    if (this->ptr_ != nullptr)
        { this->ptr_->strong_count.fetch_add(1, std::memory_order::relaxed); }
  }
  strong_compact_ptr(strong_compact_ptr&& rhs) noexcept
      : indirect_ptr<Storage>(std::exchange(rhs.ptr_, nullptr)) {}
  ~strong_compact_ptr() noexcept(std::is_nothrow_destructible_v<T>) {
    if (this->ptr_ != nullptr && this->ptr_->strong_count.fetch_sub(
        1, std::memory_order::acq_rel) == 1) {
      std::destroy_at(operator->());
      if (this->ptr_->weak_count.fetch_sub(
          1u, std::memory_order::release) == 1) {
        deallocate(this->ptr_->alloc, this->ptr_);
      }
    }
  }
  explicit operator bool() const noexcept { return this->ptr_ != nullptr; }
  T* operator->() noexcept
      { return std::launder(reinterpret_cast<T*>(&this->ptr_->value)); }
  const T* operator->() const noexcept
      { return std::launder(reinterpret_cast<const T*>(&this->ptr_->value)); }
  T& operator*() & noexcept { return *operator->(); }
  const T& operator*() const& noexcept { return *operator->(); }
  T&& operator*() && noexcept { return std::move(*operator->()); }
  const T&& operator*() const&& noexcept { return std::move(*operator->()); }

 private:
  explicit strong_compact_ptr(Storage* ptr) noexcept
      : indirect_ptr<Storage>(ptr) {}
};
template <class T, class Alloc>
class weak_compact_ptr {
 public:
  weak_compact_ptr(const strong_compact_ptr<T, Alloc>& rhs) noexcept
      : ptr_(rhs.ptr_) {
    if (ptr_ != nullptr)
        { ptr_->weak_count.fetch_add(1, std::memory_order::relaxed); }
  }
  weak_compact_ptr(const weak_compact_ptr& rhs) noexcept : ptr_(rhs.ptr_) {
    if (ptr_ != nullptr)
        { ptr_->weak_count.fetch_add(1, std::memory_order::relaxed); }
  }
  weak_compact_ptr(weak_compact_ptr&& rhs) noexcept
      : ptr_(std::exchange(rhs.ptr_, nullptr)) {}
  ~weak_compact_ptr() noexcept {
    if (ptr_ != nullptr && ptr_->weak_count.fetch_sub(
        1u, std::memory_order::acq_rel) == 1) {
      deallocate(ptr_->alloc, ptr_);
    }
  }
  strong_compact_ptr<T, Alloc> lock() const noexcept {
    long ref_count = ptr_->strong_count.load(std::memory_order::relaxed);
    do {
      if (ref_count == 0) { return strong_compact_ptr<T, Alloc>{nullptr}; }
    } while (!ptr_->strong_count.compare_exchange_weak(
        ref_count, ref_count + 1, std::memory_order::relaxed));
    return strong_compact_ptr<T, Alloc>{ptr_};
  }

 private:
  strong_weak_compact_ptr_storage<T, Alloc>* ptr_;
};

struct weak_conversion_dispatch;
template <class... Cs>
struct weak_ownership_support_traits_impl : inapplicable_traits {};
template <class... Cs>
    requires(std::is_same_v<
        typename Cs::dispatch_type, weak_conversion_dispatch> || ...)
struct weak_ownership_support_traits_impl<Cs...> : applicable_traits {};
template <class F>
struct weak_ownership_support_traits : instantiated_t<
    weak_ownership_support_traits_impl, typename F::convention_types> {};

template <class F, class T, class Alloc, class... Args>
constexpr proxy<F> allocate_proxy_impl(const Alloc& alloc, Args&&... args) {
  if constexpr (proxiable<allocated_ptr<T, Alloc>, F>) {
    return proxy<F>{std::in_place_type<allocated_ptr<T, Alloc>>,
        alloc, std::forward<Args>(args)...};
  } else {
    return proxy<F>{std::in_place_type<compact_ptr<T, Alloc>>,
        alloc, std::forward<Args>(args)...};
  }
}
template <class F, class T, class... Args>
constexpr proxy<F> make_proxy_impl(Args&&... args) {
  if constexpr (proxiable<inplace_ptr<T>, F>) {
    return proxy<F>{std::in_place_type<inplace_ptr<T>>, std::in_place,
        std::forward<Args>(args)...};
  } else {
    return allocate_proxy_impl<F, T>(
        std::allocator<void>{}, std::forward<Args>(args)...);
  }
}
template <class F, class T, class Alloc, class... Args>
constexpr proxy<F> allocate_proxy_shared_impl(
    const Alloc& alloc, Args&&... args) {
  if constexpr (weak_ownership_support_traits<F>::applicable) {
    return proxy<F>{std::in_place_type<strong_compact_ptr<T, Alloc>>,
        alloc, std::forward<Args>(args)...};
  } else {
    return proxy<F>{std::in_place_type<shared_compact_ptr<T, Alloc>>,
        alloc, std::forward<Args>(args)...};
  }
}
template <class F, class T, class... Args>
constexpr proxy<F> make_proxy_shared_impl(Args&&... args) {
  return allocate_proxy_shared_impl<F, T>(
      std::allocator<void>{}, std::forward<Args>(args)...);
}
#endif  // __STDC_HOSTED__

}  // namespace details

template <facade F> struct observer_facade;
template <facade F> using proxy_view = proxy<observer_facade<F>>;

template <facade F> struct weak_facade;
template <facade F> using weak_proxy = proxy<weak_facade<F>>;

template <class T, class F>
concept inplace_proxiable_target = proxiable<details::inplace_ptr<T>, F>;

template <class T, class F>
concept proxiable_target = proxiable<
    details::observer_ptr<T&, const T&, T&&, const T&&>, observer_facade<F>>;

template <facade F, class T, class... Args>
constexpr proxy<F> make_proxy_inplace(Args&&... args)
    noexcept(std::is_nothrow_constructible_v<T, Args...>)
    requires(std::is_constructible_v<T, Args...>) {
  return proxy<F>{std::in_place_type<details::inplace_ptr<T>>, std::in_place,
      std::forward<Args>(args)...};
}
template <facade F, class T, class U, class... Args>
constexpr proxy<F> make_proxy_inplace(
    std::initializer_list<U> il, Args&&... args)
    noexcept(std::is_nothrow_constructible_v<
        T, std::initializer_list<U>&, Args...>)
    requires(std::is_constructible_v<T, std::initializer_list<U>&, Args...>) {
  return proxy<F>{std::in_place_type<details::inplace_ptr<T>>, std::in_place,
      il, std::forward<Args>(args)...};
}
template <facade F, class T>
constexpr proxy<F> make_proxy_inplace(T&& value)
    noexcept(std::is_nothrow_constructible_v<std::decay_t<T>, T>)
    requires(std::is_constructible_v<std::decay_t<T>, T>) {
  return proxy<F>{std::in_place_type<details::inplace_ptr<std::decay_t<T>>>,
      std::in_place, std::forward<T>(value)};
}

template <facade F, class T>
constexpr proxy_view<F> make_proxy_view(T& value) noexcept {
  return proxy_view<F>{
      details::observer_ptr<T&, const T&, T&&, const T&&>{value}};
}

#if __STDC_HOSTED__
template <facade F, class T, class Alloc, class... Args>
constexpr proxy<F> allocate_proxy(const Alloc& alloc, Args&&... args)
    requires(std::is_constructible_v<T, Args...>) {
  return details::allocate_proxy_impl<F, T>(alloc, std::forward<Args>(args)...);
}
template <facade F, class T, class Alloc, class U, class... Args>
constexpr proxy<F> allocate_proxy(
    const Alloc& alloc, std::initializer_list<U> il, Args&&... args)
    requires(std::is_constructible_v<T, std::initializer_list<U>&, Args...>) {
  return details::allocate_proxy_impl<F, T>(
      alloc, il, std::forward<Args>(args)...);
}
template <facade F, class Alloc, class T>
constexpr proxy<F> allocate_proxy(const Alloc& alloc, T&& value)
    requires(std::is_constructible_v<std::decay_t<T>, T>) {
  return details::allocate_proxy_impl<F, std::decay_t<T>>(
      alloc, std::forward<T>(value));
}
template <facade F, class T, class... Args>
constexpr proxy<F> make_proxy(Args&&... args)
    requires(std::is_constructible_v<T, Args...>)
    { return details::make_proxy_impl<F, T>(std::forward<Args>(args)...); }
template <facade F, class T, class U, class... Args>
constexpr proxy<F> make_proxy(std::initializer_list<U> il, Args&&... args)
    requires(std::is_constructible_v<T, std::initializer_list<U>&, Args...>)
    { return details::make_proxy_impl<F, T>(il, std::forward<Args>(args)...); }
template <facade F, class T>
constexpr proxy<F> make_proxy(T&& value)
    requires(std::is_constructible_v<std::decay_t<T>, T>) {
  return details::make_proxy_impl<F, std::decay_t<T>>(std::forward<T>(value));
}

template <facade F, class T, class Alloc, class... Args>
constexpr proxy<F> allocate_proxy_shared(const Alloc& alloc, Args&&... args)
    requires(std::is_constructible_v<T, Args...>) {
  return details::allocate_proxy_shared_impl<F, T>(
      alloc, std::forward<Args>(args)...);
}
template <facade F, class T, class Alloc, class U, class... Args>
constexpr proxy<F> allocate_proxy_shared(
    const Alloc& alloc, std::initializer_list<U> il, Args&&... args)
    requires(std::is_constructible_v<T, std::initializer_list<U>&, Args...>) {
  return details::allocate_proxy_shared_impl<F, T>(
      alloc, il, std::forward<Args>(args)...);
}
template <facade F, class Alloc, class T>
constexpr proxy<F> allocate_proxy_shared(const Alloc& alloc, T&& value)
    requires(std::is_constructible_v<std::decay_t<T>, T>) {
  return details::allocate_proxy_shared_impl<F, std::decay_t<T>>(
      alloc, std::forward<T>(value));
}
template <facade F, class T, class... Args>
constexpr proxy<F> make_proxy_shared(Args&&... args)
    requires(std::is_constructible_v<T, Args...>) {
  return details::make_proxy_shared_impl<F, T>(std::forward<Args>(args)...);
}
template <facade F, class T, class U, class... Args>
constexpr proxy<F> make_proxy_shared(
    std::initializer_list<U> il, Args&&... args)
    requires(std::is_constructible_v<T, std::initializer_list<U>&, Args...>) {
  return details::make_proxy_shared_impl<F, T>(il, std::forward<Args>(args)...);
}
template <facade F, class T>
constexpr proxy<F> make_proxy_shared(T&& value)
    requires(std::is_constructible_v<std::decay_t<T>, T>) {
  return details::make_proxy_shared_impl<F, std::decay_t<T>>(
      std::forward<T>(value));
}
#endif  // __STDC_HOSTED__

#if __cpp_rtti >= 199711L
class bad_proxy_cast : public std::bad_cast {
 public:
  char const* what() const noexcept override
      { return "pro::v3::bad_proxy_cast"; }
};
#endif  // __cpp_rtti >= 199711L

#define ___PRO3_DIRECT_FUNC_IMPL(...) \
    noexcept(noexcept(__VA_ARGS__)) requires(requires { __VA_ARGS__; }) \
    { return __VA_ARGS__; }

#define ___PRO3_DEF_MEM_ACCESSOR_TEMPLATE(__MACRO, ...) \
    template <class __F, bool __IsDirect, class __D, class... __Os> \
    struct ___PRO3_ENFORCE_EBO accessor { accessor() = delete; }; \
    template <class __F, bool __IsDirect, class __D, class... __Os> \
        requires(sizeof...(__Os) > 1u && (::std::is_constructible_v< \
            accessor<__F, __IsDirect, __D, __Os>> && ...)) \
    struct accessor<__F, __IsDirect, __D, __Os...> \
        : accessor<__F, __IsDirect, __D, __Os>... \
        { using accessor<__F, __IsDirect, __D, __Os>::__VA_ARGS__...; }; \
    __MACRO(, ::pro::v3::access_proxy<__F>(*this), __VA_ARGS__); \
    __MACRO(noexcept, ::pro::v3::access_proxy<__F>(*this), __VA_ARGS__); \
    __MACRO(&, ::pro::v3::access_proxy<__F>(*this), __VA_ARGS__); \
    __MACRO(& noexcept, ::pro::v3::access_proxy<__F>(*this), __VA_ARGS__); \
    __MACRO(&&, ::pro::v3::access_proxy<__F>(::std::move(*this)), \
        __VA_ARGS__); \
    __MACRO(&& noexcept, ::pro::v3::access_proxy<__F>(::std::move(*this)), \
        __VA_ARGS__); \
    __MACRO(const, ::pro::v3::access_proxy<__F>(*this), __VA_ARGS__); \
    __MACRO(const noexcept, ::pro::v3::access_proxy<__F>(*this), \
        __VA_ARGS__); \
    __MACRO(const&, ::pro::v3::access_proxy<__F>(*this), __VA_ARGS__); \
    __MACRO(const& noexcept, ::pro::v3::access_proxy<__F>(*this), \
        __VA_ARGS__); \
    __MACRO(const&&, ::pro::v3::access_proxy<__F>(::std::move(*this)), \
        __VA_ARGS__); \
    __MACRO(const&& noexcept, ::pro::v3::access_proxy<__F>( \
        ::std::move(*this)), __VA_ARGS__);

#define ___PRO3_ADL_ARG ::pro::v3::details::adl_accessor_arg_t<__F, __IsDirect>
#define ___PRO3_DEF_FREE_ACCESSOR_TEMPLATE(__MACRO, ...) \
    template <class __F, bool __IsDirect, class __D, class... __Os> \
    struct ___PRO3_ENFORCE_EBO accessor { accessor() = delete; }; \
    template <class __F, bool __IsDirect, class __D, class... __Os> \
        requires(sizeof...(__Os) > 1u && (::std::is_constructible_v< \
            accessor<__F, __IsDirect, __D, __Os>> && ...)) \
    struct accessor<__F, __IsDirect, __D, __Os...> \
        : accessor<__F, __IsDirect, __D, __Os>... {}; \
    __MACRO(,, ___PRO3_ADL_ARG& __self, ::pro::v3::access_proxy<__F>(__self), \
        __VA_ARGS__); \
    __MACRO(noexcept, noexcept, ___PRO3_ADL_ARG& __self, \
        ::pro::v3::access_proxy<__F>(__self), __VA_ARGS__); \
    __MACRO(&,, ___PRO3_ADL_ARG& __self, ::pro::v3::access_proxy<__F>(__self), \
        __VA_ARGS__); \
    __MACRO(& noexcept, noexcept, ___PRO3_ADL_ARG& __self, \
        ::pro::v3::access_proxy<__F>(__self), __VA_ARGS__); \
    __MACRO(&&,, ___PRO3_ADL_ARG&& __self, ::pro::v3::access_proxy<__F>( \
        ::std::forward<decltype(__self)>(__self)), __VA_ARGS__); \
    __MACRO(&& noexcept, noexcept, ___PRO3_ADL_ARG&& __self, \
        ::pro::v3::access_proxy<__F>( \
            ::std::forward<decltype(__self)>(__self)), __VA_ARGS__); \
    __MACRO(const,, const ___PRO3_ADL_ARG& __self, \
        ::pro::v3::access_proxy<__F>(__self), __VA_ARGS__); \
    __MACRO(const noexcept, noexcept, const ___PRO3_ADL_ARG& __self, \
        ::pro::v3::access_proxy<__F>(__self), __VA_ARGS__); \
    __MACRO(const&,, const ___PRO3_ADL_ARG& __self, \
        ::pro::v3::access_proxy<__F>(__self), __VA_ARGS__); \
    __MACRO(const& noexcept, noexcept, const ___PRO3_ADL_ARG& __self, \
        ::pro::v3::access_proxy<__F>(__self), __VA_ARGS__); \
    __MACRO(const&&,, const ___PRO3_ADL_ARG&& __self, \
        ::pro::v3::access_proxy<__F>( \
            ::std::forward<decltype(__self)>(__self)), __VA_ARGS__); \
    __MACRO(const&& noexcept, noexcept, const ___PRO3_ADL_ARG&& __self, \
        ::pro::v3::access_proxy<__F>( \
            ::std::forward<decltype(__self)>(__self)), __VA_ARGS__);

#define ___PRO3_GEN_DEBUG_SYMBOL_FOR_MEM_ACCESSOR(...) \
    ___PRO3_DEBUG( \
        accessor() noexcept { ::std::ignore = &accessor::__VA_ARGS__; })

#define ___PRO3_EXPAND_IMPL(__X) __X
#define ___PRO3_EXPAND_MACRO_IMPL(__MACRO, __1, __2, __3, __NAME, ...) \
    __MACRO##_##__NAME
#define ___PRO3_EXPAND_MACRO(__MACRO, ...) \
    ___PRO3_EXPAND_IMPL(___PRO3_EXPAND_MACRO_IMPL( \
        __MACRO, __VA_ARGS__, 3, 2)(__VA_ARGS__))

#define ___PRO3_DEF_MEM_ACCESSOR(__Q, __SELF, ...) \
    template <class __F, bool __IsDirect, class __D, class __R, \
        class... __Args> \
    struct accessor<__F, __IsDirect, __D, __R(__Args...) __Q> { \
      ___PRO3_GEN_DEBUG_SYMBOL_FOR_MEM_ACCESSOR(__VA_ARGS__) \
      __R __VA_ARGS__(__Args... __args) __Q { \
        return ::pro::v3::proxy_invoke<__IsDirect, __D, __R(__Args...) __Q>( \
            __SELF, ::std::forward<__Args>(__args)...); \
      } \
    }
#define ___PRO3_DEF_MEM_DISPATCH_IMPL(__NAME, __FUNC, __FNAME) \
    struct __NAME { \
      template <class __T, class... __Args> \
      ___PRO3_STATIC_CALL(decltype(auto), __T&& __self, __Args&&... __args) \
          ___PRO3_DIRECT_FUNC_IMPL(::std::forward<__T>(__self) \
              .__FUNC(::std::forward<__Args>(__args)...)) \
      ___PRO3_DEF_MEM_ACCESSOR_TEMPLATE(___PRO3_DEF_MEM_ACCESSOR, __FNAME) \
    }
#define ___PRO3_DEF_MEM_DISPATCH_2(__NAME, __FUNC) \
    ___PRO3_DEF_MEM_DISPATCH_IMPL(__NAME, __FUNC, __FUNC)
#define ___PRO3_DEF_MEM_DISPATCH_3(__NAME, __FUNC, __FNAME) \
    ___PRO3_DEF_MEM_DISPATCH_IMPL(__NAME, __FUNC, __FNAME)
#define PRO3_DEF_MEM_DISPATCH(__NAME, ...) \
    ___PRO3_EXPAND_MACRO(___PRO3_DEF_MEM_DISPATCH, __NAME, __VA_ARGS__)

#define ___PRO3_DEF_FREE_ACCESSOR(__Q, __NE, __SELF_ARG, __SELF, ...) \
    template <class __F, bool __IsDirect, class __D, class __R, \
        class... __Args> \
    struct accessor<__F, __IsDirect, __D, __R(__Args...) __Q> { \
      friend __R __VA_ARGS__(__SELF_ARG, __Args... __args) __NE { \
        return ::pro::v3::proxy_invoke<__IsDirect, __D, __R(__Args...) __Q>( \
            __SELF, ::std::forward<__Args>(__args)...); \
      } \
___PRO3_DEBUG( \
      accessor() noexcept { ::std::ignore = &_symbol_guard; } \
    \
     private: \
      static inline __R _symbol_guard(__SELF_ARG, __Args... __args) __NE { \
        return __VA_ARGS__(::std::forward<decltype(__self)>(__self), \
            ::std::forward<__Args>(__args)...); \
      } \
) \
    }
#define ___PRO3_DEF_FREE_DISPATCH_IMPL(__NAME, __FUNC, __FNAME) \
    struct __NAME { \
      template <class __T, class... __Args> \
      ___PRO3_STATIC_CALL(decltype(auto), __T&& __self, __Args&&... __args) \
          ___PRO3_DIRECT_FUNC_IMPL(__FUNC(::std::forward<__T>(__self), \
              ::std::forward<__Args>(__args)...)) \
      ___PRO3_DEF_FREE_ACCESSOR_TEMPLATE(___PRO3_DEF_FREE_ACCESSOR, __FNAME) \
    }
#define ___PRO3_DEF_FREE_DISPATCH_2(__NAME, __FUNC) \
    ___PRO3_DEF_FREE_DISPATCH_IMPL(__NAME, __FUNC, __FUNC)
#define ___PRO3_DEF_FREE_DISPATCH_3(__NAME, __FUNC, __FNAME) \
    ___PRO3_DEF_FREE_DISPATCH_IMPL(__NAME, __FUNC, __FNAME)
#define PRO3_DEF_FREE_DISPATCH(__NAME, ...) \
    ___PRO3_EXPAND_MACRO(___PRO3_DEF_FREE_DISPATCH, __NAME, __VA_ARGS__)

#define ___PRO3_DEF_FREE_AS_MEM_DISPATCH_IMPL(__NAME, __FUNC, __FNAME) \
    struct __NAME { \
      template <class __T, class... __Args> \
      ___PRO3_STATIC_CALL(decltype(auto), __T&& __self, __Args&&... __args) \
          ___PRO3_DIRECT_FUNC_IMPL(__FUNC(::std::forward<__T>(__self), \
              ::std::forward<__Args>(__args)...)) \
      ___PRO3_DEF_MEM_ACCESSOR_TEMPLATE(___PRO3_DEF_MEM_ACCESSOR, __FNAME) \
    }
#define ___PRO3_DEF_FREE_AS_MEM_DISPATCH_2(__NAME, __FUNC) \
    ___PRO3_DEF_FREE_AS_MEM_DISPATCH_IMPL(__NAME, __FUNC, __FUNC)
#define ___PRO3_DEF_FREE_AS_MEM_DISPATCH_3(__NAME, __FUNC, __FNAME) \
    ___PRO3_DEF_FREE_AS_MEM_DISPATCH_IMPL(__NAME, __FUNC, __FNAME)
#define PRO3_DEF_FREE_AS_MEM_DISPATCH(__NAME, ...) \
    ___PRO3_EXPAND_MACRO(___PRO3_DEF_FREE_AS_MEM_DISPATCH, __NAME, __VA_ARGS__)

namespace details {

template <class F, bool IsDirect>
using adl_accessor_arg_t =
    std::conditional_t<IsDirect, proxy<F>, proxy_indirect_accessor<F>>;

#define ___PRO_DEF_CAST_ACCESSOR(Q, SELF, ...) \
    template <class __F, bool __IsDirect, class __D, class T> \
    struct accessor<__F, __IsDirect, __D, T() Q> { \
      ___PRO3_GEN_DEBUG_SYMBOL_FOR_MEM_ACCESSOR(operator T) \
      explicit(Expl) operator T() Q { \
        if constexpr (Nullable) { \
          if (!SELF.has_value()) { return nullptr; } \
        } \
        return proxy_invoke<__IsDirect, __D, T() Q>(SELF); \
      } \
    }
template <bool Expl, bool Nullable>
struct cast_dispatch_base {
  ___PRO3_DEF_MEM_ACCESSOR_TEMPLATE(___PRO_DEF_CAST_ACCESSOR,
      operator typename overload_traits<__Os>::return_type)
};
#undef ___PRO_DEF_CAST_ACCESSOR

struct upward_conversion_dispatch : cast_dispatch_base<false, true> {
  template <class T>
  ___PRO3_STATIC_CALL(T&&, T&& self) noexcept { return std::forward<T>(self); }
};

template <class T>
struct explicit_conversion_adapter {
  explicit explicit_conversion_adapter(T&& value) noexcept
      : value_(std::forward<T>(value)) {}
  explicit_conversion_adapter(const explicit_conversion_adapter&) = delete;

  template <class U>
  operator U() const noexcept(std::is_nothrow_constructible_v<U, T>)
      requires(std::is_constructible_v<U, T>)
      { return U{std::forward<T>(value_)}; }

 private:
  T&& value_;
};

constexpr std::size_t invalid_size = std::numeric_limits<std::size_t>::max();
constexpr constraint_level invalid_cl = static_cast<constraint_level>(
    std::numeric_limits<std::underlying_type_t<constraint_level>>::min());
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
  if (value.max_size > max_size) { value.max_size = max_size; }
  if (value.max_align > max_align) { value.max_align = max_align; }
  return value;
}
consteval auto make_copyable(proxiable_ptr_constraints value,
    constraint_level cl) {
  if (value.copyability < cl) { value.copyability = cl; }
  return value;
}
consteval auto make_relocatable(proxiable_ptr_constraints value,
    constraint_level cl) {
  if (value.relocatability < cl) { value.relocatability = cl; }
  return value;
}
consteval auto make_destructible(proxiable_ptr_constraints value,
    constraint_level cl) {
  if (value.destructibility < cl) { value.destructibility = cl; }
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
consteval std::size_t max_align_of(std::size_t value) {
  value &= ~value + 1u;
  return value < alignof(std::max_align_t) ? value : alignof(std::max_align_t);
}

template <class SFINAE, class T, class F, bool IsDirect, class... Args>
struct accessor_instantiation_traits : std::type_identity<void> {};
template <class T, class F, bool IsDirect, class... Args>
struct accessor_instantiation_traits<std::void_t<typename T::template accessor<
    F, IsDirect, T, Args...>>, T, F, IsDirect, Args...>
    : std::type_identity<typename T::template accessor<
          F, IsDirect, T, Args...>> {};
template <class T, class F, bool IsDirect, class... Args>
using instantiated_accessor_t =
    typename accessor_instantiation_traits<void, T, F, IsDirect, Args...>::type;

template <bool IsDirect, class D, class... Os>
struct conv_impl {
  static constexpr bool is_direct = IsDirect;
  using dispatch_type = D;
  using overload_types = std::tuple<Os...>;
  template <class F>
  using accessor = instantiated_accessor_t<
      D, F, IsDirect, substituted_overload_t<Os, F>...>;
};
template <bool IsDirect, class R>
struct refl_impl {
  static constexpr bool is_direct = IsDirect;
  using reflector_type = R;
  template <class F>
  using accessor = instantiated_accessor_t<R, F, IsDirect>;
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

template <bool IsDirect, class D>
struct merge_conv_traits
    { template <class... Os> using type = conv_impl<IsDirect, D, Os...>; };
template <class C1, class C2>
using merge_conv_t = instantiated_t<
    merge_conv_traits<C1::is_direct, typename C1::dispatch_type>::template type,
    merge_tuple_t<typename C1::overload_types, typename C2::overload_types>>;

template <class Cs1, class C2, class C> struct add_conv_reduction;
template <class... Cs1, class C2, class... Cs3, class C>
struct add_conv_reduction<std::tuple<Cs1...>, std::tuple<C2, Cs3...>, C>
    : add_conv_reduction<std::tuple<Cs1..., C2>, std::tuple<Cs3...>, C> {};
template <class... Cs1, class C2, class... Cs3, class C>
    requires(C::is_direct == C2::is_direct && std::is_same_v<
        typename C::dispatch_type, typename C2::dispatch_type>)
struct add_conv_reduction<std::tuple<Cs1...>, std::tuple<C2, Cs3...>, C>
    : std::type_identity<std::tuple<Cs1..., merge_conv_t<C2, C>, Cs3...>> {};
template <class... Cs, class C>
struct add_conv_reduction<std::tuple<Cs...>, std::tuple<>, C>
    : std::type_identity<std::tuple<Cs..., merge_conv_t<
          conv_impl<C::is_direct, typename C::dispatch_type>, C>>> {};
template <class Cs, class C>
using add_conv_t = typename add_conv_reduction<std::tuple<>, Cs, C>::type;

template <class F, constraint_level CL>
using copy_conversion_overload =
    proxy<F>() const& noexcept(CL >= constraint_level::nothrow);
template <class F, constraint_level CL>
using move_conversion_overload =
    proxy<F>() && noexcept(CL >= constraint_level::nothrow);
template <class Cs, class F, constraint_level CCL, constraint_level RCL>
struct add_upward_conversion_conv
    : std::type_identity<add_conv_t<Cs, conv_impl<true,
          upward_conversion_dispatch, copy_conversion_overload<F, CCL>,
          move_conversion_overload<F, RCL>>>> {};
template <class Cs, class F, constraint_level RCL>
struct add_upward_conversion_conv<Cs, F, constraint_level::none, RCL>
    : std::type_identity<add_conv_t<Cs, conv_impl<true,
          upward_conversion_dispatch, move_conversion_overload<F, RCL>>>> {};
template <class Cs, class F, constraint_level CCL>
struct add_upward_conversion_conv<Cs, F, CCL, constraint_level::none>
    : std::type_identity<add_conv_t<Cs, conv_impl<true,
          upward_conversion_dispatch, copy_conversion_overload<F, CCL>>>> {};
template <class Cs, class F>
struct add_upward_conversion_conv<
    Cs, F, constraint_level::none, constraint_level::none>
    : std::type_identity<Cs> {};

template <class Cs1, class... Cs2>
using merge_conv_tuple_t = recursive_reduction_t<add_conv_t, Cs1, Cs2...>;
template <class Cs, class F, bool WithUpwardConversion>
using merge_facade_conv_t = typename add_upward_conversion_conv<
    instantiated_t<merge_conv_tuple_t, typename F::convention_types, Cs>, F,
    WithUpwardConversion ? F::constraints.copyability : constraint_level::none,
    (WithUpwardConversion &&
        F::constraints.copyability != constraint_level::trivial) ?
        F::constraints.relocatability : constraint_level::none>::type;

struct proxy_view_dispatch : cast_dispatch_base<false, true> {
  template <class T>
  ___PRO3_STATIC_CALL(auto, T& value) noexcept
      requires(requires { { std::addressof(*value) } noexcept; }) {
    return observer_ptr<decltype(*value), decltype(*std::as_const(value)),
        decltype(*std::move(value)), decltype(*std::move(std::as_const(value)))>
        {*value};
  }
};

template <class O>
using observer_upward_conversion_overload = proxy_view<typename proxy_traits<
    typename overload_traits<O>::return_type>::facade_type>() const noexcept;

template <class O, class I>
struct observer_upward_conversion_conv_reduction : std::type_identity<O> {};
template <class... Os, class O>
    requires(!std::is_same_v<Os, observer_upward_conversion_overload<O>> && ...)
struct observer_upward_conversion_conv_reduction<
    conv_impl<true, upward_conversion_dispatch, Os...>, O>
    : std::type_identity<conv_impl<true, upward_conversion_dispatch, Os...,
          observer_upward_conversion_overload<O>>> {};
template <class O, class I>
using observer_upward_conversion_conv_reduction_t =
    typename observer_upward_conversion_conv_reduction<O, I>::type;
template <class... Os>
using observer_upward_conversion_conv =
    recursive_reduction_t<observer_upward_conversion_conv_reduction_t,
        conv_impl<true, upward_conversion_dispatch>, Os...>;

template <class D, class F, class... Os>
using observer_indirect_conv =
    conv_impl<false, D, substituted_overload_t<Os, F>...>;

template <class C, class F>
struct observer_conv_traits : inapplicable_traits {};
template <class C, class F>
    requires(C::is_direct &&
        std::is_same_v<typename C::dispatch_type, upward_conversion_dispatch>)
struct observer_conv_traits<C, F>
    : applicable_traits, std::type_identity<instantiated_t<
          observer_upward_conversion_conv, typename C::overload_types>> {};
template <class C, class F> requires(!C::is_direct)
struct observer_conv_traits<C, F>
    : applicable_traits, std::type_identity<instantiated_t<
          observer_indirect_conv, typename C::overload_types,
          typename C::dispatch_type, F>> {};

template <class F, class O, class I>
struct observer_conv_reduction : std::type_identity<O> {};
template <class F, class... Cs, class I>
    requires(observer_conv_traits<I, F>::applicable)
struct observer_conv_reduction<F, std::tuple<Cs...>, I>
    : std::type_identity<
          std::tuple<Cs..., typename observer_conv_traits<I, F>::type>> {};
template <class F>
struct observer_conv_reduction_helper {
  template <class O, class I>
  using type = typename observer_conv_reduction<F, O, I>::type;
};

template <class O, class I>
struct observer_refl_reduction : std::type_identity<O> {};
template <class... Rs, class R> requires(!R::is_direct)
struct observer_refl_reduction<std::tuple<Rs...>, R>
    : std::type_identity<std::tuple<Rs..., R>> {};
template <class O, class I>
using observer_refl_reduction_t = typename observer_refl_reduction<O, I>::type;

template <class F, class... Cs>
struct observer_facade_conv_impl {
  using convention_types = recursive_reduction_t<
      observer_conv_reduction_helper<F>::template type, std::tuple<>, Cs...>;
};
template <class... Rs>
struct observer_facade_refl_impl {
  using reflection_types = recursive_reduction_t<
      observer_refl_reduction_t, std::tuple<>, Rs...>;
};

template <class F>
using proxy_view_overload = proxy_view<F>() noexcept;

template <std::size_t N>
struct sign {
  consteval sign(const char (&str)[N])
      { for (std::size_t i = 0; i < N; ++i) { value[i] = str[i]; } }

  char value[N];
};
template <std::size_t N>
sign(const char (&str)[N]) -> sign<N>;

struct weak_conversion_dispatch : cast_dispatch_base<false, true> {
  template <class P>
  ___PRO3_STATIC_CALL(auto, const P& self) noexcept
      requires(
          requires(const typename P::weak_type& w)
              { { w.lock() } noexcept -> std::same_as<P>; } &&
          std::is_convertible_v<const P&, typename P::weak_type>)
      { return typename P::weak_type{self}; }
};
template <class F>
using weak_conversion_overload = weak_proxy<F>() const noexcept;

template <class P>
class nullable_ptr_adapter {
 public:
  explicit nullable_ptr_adapter(P&& ptr) : ptr_(std::move(ptr)) {}
  nullable_ptr_adapter(const nullable_ptr_adapter&) = delete;
  template <class F>
  operator proxy<F>() noexcept {
    if (static_cast<bool>(ptr_)) {
      return std::move(ptr_);
    } else {
      return nullptr;
    }
  }

 private:
  P ptr_;
};
template <class P>
auto weak_lock_impl(const P& self) noexcept
    requires(requires { static_cast<bool>(self.lock()); })
    { return nullable_ptr_adapter{self.lock()}; }
PRO3_DEF_FREE_AS_MEM_DISPATCH(weak_mem_lock, weak_lock_impl, lock);

#if __STDC_HOSTED__
template <class CharT> struct format_overload_traits;
template <>
struct format_overload_traits<char>
    : std::type_identity<std::format_context::iterator(
          std::string_view spec, std::format_context& fc) const> {};
template <>
struct format_overload_traits<wchar_t>
    : std::type_identity<std::wformat_context::iterator(
          std::wstring_view spec, std::wformat_context& fc) const> {};
template <class CharT>
using format_overload_t = typename format_overload_traits<CharT>::type;

struct format_dispatch {
  // Note: This function requires std::formatter<T, CharT> to be well-formed.
  // However, the standard did not provide such facility before C++23. In the
  // "required" clause of this function, std::formattable (C++23) is preferred
  // when available. Otherwise, when building with C++20, we simply check
  // whether std::formatter<T, CharT> is a disabled specialization of
  // std::formatter by std::is_default_constructible_v as per
  // [format.formatter.spec].
  template <class T, class CharT, class OutIt>
  ___PRO3_STATIC_CALL(OutIt, const T& self, std::basic_string_view<CharT> spec,
      std::basic_format_context<OutIt, CharT>& fc)
      requires(
#if __cpp_lib_format_ranges >= 202207L
          std::formattable<T, CharT>
#else
          std::is_default_constructible_v<std::formatter<T, CharT>>
#endif  // __cpp_lib_format_ranges >= 202207L
      ) {
    std::formatter<T, CharT> impl;
    {
      std::basic_format_parse_context<CharT> pc{spec};
      impl.parse(pc);
    }
    return impl.format(self, fc);
  }
};
#endif  // __STDC_HOSTED__

#if __cpp_rtti >= 199711L
struct proxy_cast_context {
  const std::type_info* type_ptr;
  bool is_ref;
  bool is_const;
  void* result_ptr;
};

struct proxy_cast_dispatch;
template <class F, bool IsDirect, class D, class O>
struct proxy_cast_accessor_impl {
  using _Self = add_qualifier_t<
      adl_accessor_arg_t<F, IsDirect>, overload_traits<O>::qualifier>;
  template <class T>
  friend T proxy_cast(_Self self) {
    static_assert(!std::is_rvalue_reference_v<T>);
    if (!access_proxy<F>(self).has_value()) { ___PRO_THROW(bad_proxy_cast{}); }
    if constexpr (std::is_lvalue_reference_v<T>) {
      using U = std::remove_reference_t<T>;
      void* result = nullptr;
      proxy_cast_context ctx{.type_ptr = &typeid(T), .is_ref = true,
          .is_const = std::is_const_v<U>, .result_ptr = &result};
      proxy_invoke<IsDirect, D, O>(
          access_proxy<F>(std::forward<_Self>(self)), ctx);
      if (result == nullptr) { ___PRO_THROW(bad_proxy_cast{}); }
      return *static_cast<U*>(result);
    } else {
      std::optional<std::remove_const_t<T>> result;
      proxy_cast_context ctx{.type_ptr = &typeid(T), .is_ref = false,
          .is_const = false, .result_ptr = &result};
      proxy_invoke<IsDirect, D, O>(
          access_proxy<F>(std::forward<_Self>(self)), ctx);
      if (!result.has_value()) { ___PRO_THROW(bad_proxy_cast{}); }
      return std::move(*result);
    }
  }
  template <class T>
  friend T* proxy_cast(std::remove_reference_t<_Self>* self) noexcept
      requires(std::is_lvalue_reference_v<_Self>) {
    if (!access_proxy<F>(*self).has_value()) { return nullptr; }
    void* result = nullptr;
    proxy_cast_context ctx{.type_ptr = &typeid(T), .is_ref = true,
        .is_const = std::is_const_v<T>, .result_ptr = &result};
    proxy_invoke<IsDirect, D, O>(access_proxy<F>(*self), ctx);
    return static_cast<T*>(result);
  }
};

#define ___PRO_DEF_PROXY_CAST_ACCESSOR(Q, ...) \
    template <class F, bool IsDirect, class D> \
    struct accessor<F, IsDirect, D, void(proxy_cast_context) Q> \
        : proxy_cast_accessor_impl<F, IsDirect, D, \
              void(proxy_cast_context) Q> {}
struct proxy_cast_dispatch {
  template <class T>
  ___PRO3_STATIC_CALL(void, T&& self, proxy_cast_context ctx) {
    if (typeid(T) == *ctx.type_ptr) {
      if (ctx.is_ref) {
        if constexpr (std::is_lvalue_reference_v<T>) {
          if (ctx.is_const || !std::is_const_v<T>) {
            *static_cast<void**>(ctx.result_ptr) = (void*)&self;
          }
        }
      } else {
        if constexpr (std::is_constructible_v<std::decay_t<T>, T>) {
          static_cast<std::optional<std::decay_t<T>>*>(ctx.result_ptr)
              ->emplace(std::forward<T>(self));
        }
      }
    }
  }
  ___PRO3_DEF_FREE_ACCESSOR_TEMPLATE(___PRO_DEF_PROXY_CAST_ACCESSOR)
};
#undef ___PRO_DEF_PROXY_CAST_ACCESSOR

struct proxy_typeid_reflector {
  template <class T>
  constexpr explicit proxy_typeid_reflector(std::in_place_type_t<T>)
      : info(&typeid(T)) {}
  constexpr proxy_typeid_reflector(const proxy_typeid_reflector&) = default;

  template <class F, bool IsDirect, class R>
  struct accessor {
    friend const std::type_info& proxy_typeid(
        const adl_accessor_arg_t<F, IsDirect>& self) noexcept {
      const proxy<F>& p = access_proxy<F>(self);
      if (!p.has_value()) { return typeid(void); }
      const proxy_typeid_reflector& refl = proxy_reflect<IsDirect, R>(p);
      return *refl.info;
    }
___PRO3_DEBUG(
    accessor() noexcept { std::ignore = &_symbol_guard; }

   private:
    static inline const std::type_info& _symbol_guard(
        const adl_accessor_arg_t<F, IsDirect>& self) noexcept
        { return proxy_typeid(self); }
)
  };

  const std::type_info* info;
};
#endif  // __cpp_rtti >= 199711L

struct wildcard {
  wildcard() = delete;

  template <class T>
  [[noreturn]] operator T() const {
#if __cpp_lib_unreachable >= 202202L
    std::unreachable();
#else
    std::abort();
#endif  // __cpp_lib_unreachable >= 202202L
  }
};

}  // namespace details

template <class Cs, class Rs, proxiable_ptr_constraints C>
struct basic_facade_builder {
  template <class D, details::extended_overload... Os>
      requires(sizeof...(Os) > 0u)
  using add_indirect_convention = basic_facade_builder<
      details::add_conv_t<Cs, details::conv_impl<false, D, Os...>>, Rs, C>;
  template <class D, details::extended_overload... Os>
      requires(sizeof...(Os) > 0u)
  using add_direct_convention = basic_facade_builder<
      details::add_conv_t<Cs, details::conv_impl<true, D, Os...>>, Rs, C>;
  template <class D, details::extended_overload... Os>
      requires(sizeof...(Os) > 0u)
  using add_convention = add_indirect_convention<D, Os...>;
  template <class R>
  using add_indirect_reflection = basic_facade_builder<
      Cs, details::add_tuple_t<Rs, details::refl_impl<false, R>>, C>;
  template <class R>
  using add_direct_reflection = basic_facade_builder<
      Cs, details::add_tuple_t<Rs, details::refl_impl<true, R>>, C>;
  template <class R>
  using add_reflection = add_indirect_reflection<R>;
  template <facade F, bool WithUpwardConversion = false>
  using add_facade = basic_facade_builder<
      details::merge_facade_conv_t<Cs, F, WithUpwardConversion>,
      details::merge_tuple_t<Rs, typename F::reflection_types>,
      details::merge_constraints(C, F::constraints)>;
  template <std::size_t PtrSize,
      std::size_t PtrAlign = details::max_align_of(PtrSize)>
      requires(std::has_single_bit(PtrAlign) && PtrSize % PtrAlign == 0u)
  using restrict_layout = basic_facade_builder<
      Cs, Rs, details::make_restricted_layout(C, PtrSize, PtrAlign)>;
  template <constraint_level CL>
  using support_copy = basic_facade_builder<
      Cs, Rs, details::make_copyable(C, CL)>;
  template <constraint_level CL>
  using support_relocation = basic_facade_builder<
      Cs, Rs, details::make_relocatable(C, CL)>;
  template <constraint_level CL>
  using support_destruction = basic_facade_builder<
      Cs, Rs, details::make_destructible(C, CL)>;
#if __STDC_HOSTED__
  using support_format = add_convention<
      details::format_dispatch, details::format_overload_t<char>>;
  using support_wformat = add_convention<
      details::format_dispatch, details::format_overload_t<wchar_t>>;
#endif  // __STDC_HOSTED__
#if __cpp_rtti >= 199711L
  using support_indirect_rtti = basic_facade_builder<
      details::add_conv_t<Cs, details::conv_impl<false,
          details::proxy_cast_dispatch, void(details::proxy_cast_context) &,
          void(details::proxy_cast_context) const&,
          void(details::proxy_cast_context) &&>>,
      details::add_tuple_t<Rs, details::refl_impl<false,
          details::proxy_typeid_reflector>>, C>;
  using support_direct_rtti = basic_facade_builder<
      details::add_conv_t<Cs, details::conv_impl<true,
          details::proxy_cast_dispatch, void(details::proxy_cast_context) &,
          void(details::proxy_cast_context) const&,
          void(details::proxy_cast_context) &&>>,
      details::add_tuple_t<Rs, details::refl_impl<true,
          details::proxy_typeid_reflector>>, C>;
  using support_rtti = support_indirect_rtti;
#endif  // __cpp_rtti >= 199711L
  using support_view = add_direct_convention<details::proxy_view_dispatch,
      facade_aware_overload_t<details::proxy_view_overload>>;
  using support_weak = add_direct_convention<details::weak_conversion_dispatch,
      facade_aware_overload_t<details::weak_conversion_overload>>;
  using build = details::facade_impl<Cs, Rs, details::normalize(C)>;
  basic_facade_builder() = delete;
};
using facade_builder = basic_facade_builder<std::tuple<>, std::tuple<>,
    proxiable_ptr_constraints{
        .max_size = details::invalid_size,
        .max_align = details::invalid_size,
        .copyability = details::invalid_cl,
        .relocatability = details::invalid_cl,
        .destructibility = details::invalid_cl}>;

template <facade F>
struct observer_facade
    : details::instantiated_t<details::observer_facade_conv_impl,
          typename F::convention_types, F>,
      details::instantiated_t<details::observer_facade_refl_impl,
          typename F::reflection_types> {
  static constexpr proxiable_ptr_constraints constraints{
      .max_size = sizeof(void*), .max_align = alignof(void*),
      .copyability = constraint_level::trivial,
      .relocatability = constraint_level::trivial,
      .destructibility = constraint_level::trivial};
};

template <facade F>
struct weak_facade {
  using convention_types = std::tuple<details::conv_impl<
      true, details::weak_mem_lock, proxy<F>() const noexcept>>;
  using reflection_types = std::tuple<>;
  static constexpr auto constraints = F::constraints;
};

template <details::sign Sign, bool Rhs = false>
struct operator_dispatch;

#define ___PRO_DEF_LHS_LEFT_OP_ACCESSOR(Q, SELF, ...) \
    template <class __F, bool __IsDirect, class __D, class R> \
    struct accessor<__F, __IsDirect, __D, R() Q> { \
      ___PRO3_GEN_DEBUG_SYMBOL_FOR_MEM_ACCESSOR(__VA_ARGS__) \
      R __VA_ARGS__() Q { return proxy_invoke<__IsDirect, __D, R() Q>(SELF); } \
    }
#define ___PRO_DEF_LHS_ANY_OP_ACCESSOR(Q, SELF, ...) \
    template <class __F, bool __IsDirect, class __D, class R, class... Args> \
    struct accessor<__F, __IsDirect, __D, R(Args...) Q> { \
      ___PRO3_GEN_DEBUG_SYMBOL_FOR_MEM_ACCESSOR(__VA_ARGS__) \
      R __VA_ARGS__(Args... args) Q { \
        return proxy_invoke<__IsDirect, __D, R(Args...) Q>( \
            SELF, std::forward<Args>(args)...); \
      } \
    }
#define ___PRO_DEF_LHS_UNARY_OP_ACCESSOR ___PRO_DEF_LHS_ANY_OP_ACCESSOR
#define ___PRO_DEF_LHS_BINARY_OP_ACCESSOR ___PRO_DEF_LHS_ANY_OP_ACCESSOR
#define ___PRO_DEF_LHS_ALL_OP_ACCESSOR ___PRO_DEF_LHS_ANY_OP_ACCESSOR
#define ___PRO_LHS_LEFT_OP_DISPATCH_BODY_IMPL(...) \
    template <class T> \
    ___PRO3_STATIC_CALL(decltype(auto), T&& self) \
        ___PRO3_DIRECT_FUNC_IMPL(__VA_ARGS__ std::forward<T>(self))
#define ___PRO_LHS_UNARY_OP_DISPATCH_BODY_IMPL(...) \
    template <class T> \
    ___PRO3_STATIC_CALL(decltype(auto), T&& self) \
        ___PRO3_DIRECT_FUNC_IMPL(__VA_ARGS__ std::forward<T>(self)) \
    template <class T> \
    ___PRO3_STATIC_CALL(decltype(auto), T&& self, int) \
        ___PRO3_DIRECT_FUNC_IMPL(std::forward<T>(self) __VA_ARGS__)
#define ___PRO_LHS_BINARY_OP_DISPATCH_BODY_IMPL(...) \
    template <class T, class Arg> \
    ___PRO3_STATIC_CALL(decltype(auto), T&& self, Arg&& arg) \
        ___PRO3_DIRECT_FUNC_IMPL( \
            std::forward<T>(self) __VA_ARGS__ std::forward<Arg>(arg))
#define ___PRO_LHS_ALL_OP_DISPATCH_BODY_IMPL(...) \
    ___PRO_LHS_LEFT_OP_DISPATCH_BODY_IMPL(__VA_ARGS__) \
    ___PRO_LHS_BINARY_OP_DISPATCH_BODY_IMPL(__VA_ARGS__)
#define ___PRO_LHS_OP_DISPATCH_IMPL(TYPE, ...) \
    template <> \
    struct operator_dispatch<#__VA_ARGS__, false> { \
      ___PRO_LHS_##TYPE##_OP_DISPATCH_BODY_IMPL(__VA_ARGS__) \
      ___PRO3_DEF_MEM_ACCESSOR_TEMPLATE( \
          ___PRO_DEF_LHS_##TYPE##_OP_ACCESSOR, operator __VA_ARGS__) \
    };

#define ___PRO_DEF_RHS_OP_ACCESSOR(Q, NE, SELF_ARG, SELF, ...) \
    template <class __F, bool __IsDirect, class __D, class R, class Arg> \
    struct accessor<__F, __IsDirect, __D, R(Arg) Q> { \
      friend R operator __VA_ARGS__(Arg arg, SELF_ARG) NE { \
        return proxy_invoke<__IsDirect, __D, R(Arg) Q>( \
            SELF, std::forward<Arg>(arg)); \
      } \
___PRO3_DEBUG( \
      accessor() noexcept { std::ignore = &_symbol_guard; } \
    \
     private: \
      static inline R _symbol_guard(Arg arg, SELF_ARG) NE { \
        return std::forward<Arg>(arg) __VA_ARGS__ \
            std::forward<decltype(__self)>(__self); \
      } \
) \
    }
#define ___PRO_RHS_OP_DISPATCH_IMPL(...) \
    template <> \
    struct operator_dispatch<#__VA_ARGS__, true> { \
      template <class T, class Arg> \
      ___PRO3_STATIC_CALL(decltype(auto), T&& self, Arg&& arg) \
          ___PRO3_DIRECT_FUNC_IMPL( \
              std::forward<Arg>(arg) __VA_ARGS__ std::forward<T>(self)) \
      ___PRO3_DEF_FREE_ACCESSOR_TEMPLATE( \
          ___PRO_DEF_RHS_OP_ACCESSOR, __VA_ARGS__) \
    };

#define ___PRO_EXTENDED_BINARY_OP_DISPATCH_IMPL(...) \
    ___PRO_LHS_OP_DISPATCH_IMPL(ALL, __VA_ARGS__) \
    ___PRO_RHS_OP_DISPATCH_IMPL(__VA_ARGS__)

#define ___PRO_BINARY_OP_DISPATCH_IMPL(...) \
    ___PRO_LHS_OP_DISPATCH_IMPL(BINARY, __VA_ARGS__) \
    ___PRO_RHS_OP_DISPATCH_IMPL(__VA_ARGS__)

#define ___PRO_DEF_LHS_ASSIGNMENT_OP_ACCESSOR(Q, SELF, ...) \
    template <class __F, bool __IsDirect, class __D, class R, class Arg> \
    struct accessor<__F, __IsDirect, __D, R(Arg) Q> { \
      ___PRO3_GEN_DEBUG_SYMBOL_FOR_MEM_ACCESSOR(__VA_ARGS__) \
      decltype(auto) __VA_ARGS__(Arg arg) Q { \
        proxy_invoke<__IsDirect, __D, R(Arg) Q>(SELF, std::forward<Arg>(arg)); \
        if constexpr (__IsDirect) { \
          return SELF; \
        } else { \
          return *SELF; \
        } \
      } \
    }
#define ___PRO_DEF_RHS_ASSIGNMENT_OP_ACCESSOR(Q, NE, SELF_ARG, SELF, ...) \
    template <class __F, bool __IsDirect, class __D, class R, class Arg> \
    struct accessor<__F, __IsDirect, __D, R(Arg&) Q> { \
      friend Arg& operator __VA_ARGS__(Arg& arg, SELF_ARG) NE { \
        proxy_invoke<__IsDirect, __D, R(Arg&) Q>(SELF, arg); \
        return arg; \
      } \
___PRO3_DEBUG( \
      accessor() noexcept { std::ignore = &_symbol_guard; } \
    \
     private: \
      static inline Arg& _symbol_guard(Arg& arg, SELF_ARG) NE \
          { return arg __VA_ARGS__ std::forward<decltype(__self)>(__self); } \
) \
    }
#define ___PRO_ASSIGNMENT_OP_DISPATCH_IMPL(...) \
    template <> \
    struct operator_dispatch<#__VA_ARGS__, false> { \
      template <class T, class Arg> \
      ___PRO3_STATIC_CALL(decltype(auto), T&& self, Arg&& arg) \
          ___PRO3_DIRECT_FUNC_IMPL(std::forward<T>(self) __VA_ARGS__ \
              std::forward<Arg>(arg)) \
      ___PRO3_DEF_MEM_ACCESSOR_TEMPLATE(___PRO_DEF_LHS_ASSIGNMENT_OP_ACCESSOR, \
          operator __VA_ARGS__) \
    }; \
    template <> \
    struct operator_dispatch<#__VA_ARGS__, true> { \
      template <class T, class Arg> \
      ___PRO3_STATIC_CALL(decltype(auto), T&& self, Arg&& arg) \
          ___PRO3_DIRECT_FUNC_IMPL( \
              std::forward<Arg>(arg) __VA_ARGS__ std::forward<T>(self)) \
      ___PRO3_DEF_FREE_ACCESSOR_TEMPLATE( \
          ___PRO_DEF_RHS_ASSIGNMENT_OP_ACCESSOR, __VA_ARGS__) \
    };

___PRO_EXTENDED_BINARY_OP_DISPATCH_IMPL(+)
___PRO_EXTENDED_BINARY_OP_DISPATCH_IMPL(-)
___PRO_EXTENDED_BINARY_OP_DISPATCH_IMPL(*)
___PRO_BINARY_OP_DISPATCH_IMPL(/)
___PRO_BINARY_OP_DISPATCH_IMPL(%)
___PRO_LHS_OP_DISPATCH_IMPL(UNARY, ++)
___PRO_LHS_OP_DISPATCH_IMPL(UNARY, --)
___PRO_BINARY_OP_DISPATCH_IMPL(==)
___PRO_BINARY_OP_DISPATCH_IMPL(!=)
___PRO_BINARY_OP_DISPATCH_IMPL(>)
___PRO_BINARY_OP_DISPATCH_IMPL(<)
___PRO_BINARY_OP_DISPATCH_IMPL(>=)
___PRO_BINARY_OP_DISPATCH_IMPL(<=)
___PRO_BINARY_OP_DISPATCH_IMPL(<=>)
___PRO_LHS_OP_DISPATCH_IMPL(LEFT, !)
___PRO_BINARY_OP_DISPATCH_IMPL(&&)
___PRO_BINARY_OP_DISPATCH_IMPL(||)
___PRO_LHS_OP_DISPATCH_IMPL(LEFT, ~)
___PRO_EXTENDED_BINARY_OP_DISPATCH_IMPL(&)
___PRO_BINARY_OP_DISPATCH_IMPL(|)
___PRO_BINARY_OP_DISPATCH_IMPL(^)
___PRO_BINARY_OP_DISPATCH_IMPL(<<)
___PRO_BINARY_OP_DISPATCH_IMPL(>>)
___PRO_ASSIGNMENT_OP_DISPATCH_IMPL(+=)
___PRO_ASSIGNMENT_OP_DISPATCH_IMPL(-=)
___PRO_ASSIGNMENT_OP_DISPATCH_IMPL(*=)
___PRO_ASSIGNMENT_OP_DISPATCH_IMPL(/=)
___PRO_ASSIGNMENT_OP_DISPATCH_IMPL(&=)
___PRO_ASSIGNMENT_OP_DISPATCH_IMPL(|=)
___PRO_ASSIGNMENT_OP_DISPATCH_IMPL(^=)
___PRO_ASSIGNMENT_OP_DISPATCH_IMPL(<<=)
___PRO_ASSIGNMENT_OP_DISPATCH_IMPL(>>=)
___PRO_BINARY_OP_DISPATCH_IMPL(,)
___PRO_BINARY_OP_DISPATCH_IMPL(->*)

template <>
struct operator_dispatch<"()", false> {
  template <class T, class... Args>
  ___PRO3_STATIC_CALL(decltype(auto), T&& self, Args&&... args)
      ___PRO3_DIRECT_FUNC_IMPL(
          std::forward<T>(self)(std::forward<Args>(args)...))
  ___PRO3_DEF_MEM_ACCESSOR_TEMPLATE(___PRO_DEF_LHS_ANY_OP_ACCESSOR, operator())
};
template <>
struct operator_dispatch<"[]", false> {
#if __cpp_multidimensional_subscript >= 202110L
  template <class T, class... Args>
  ___PRO3_STATIC_CALL(decltype(auto), T&& self, Args&&... args)
      ___PRO3_DIRECT_FUNC_IMPL(
          std::forward<T>(self)[std::forward<Args>(args)...])
#else
  template <class T, class Arg>
  ___PRO3_STATIC_CALL(decltype(auto), T&& self, Arg&& arg)
      ___PRO3_DIRECT_FUNC_IMPL(std::forward<T>(self)[std::forward<Arg>(arg)])
#endif  // __cpp_multidimensional_subscript >= 202110L
  ___PRO3_DEF_MEM_ACCESSOR_TEMPLATE(___PRO_DEF_LHS_ANY_OP_ACCESSOR, operator[])
};

#undef ___PRO_ASSIGNMENT_OP_DISPATCH_IMPL
#undef ___PRO_DEF_RHS_ASSIGNMENT_OP_ACCESSOR
#undef ___PRO_DEF_LHS_ASSIGNMENT_OP_ACCESSOR
#undef ___PRO_BINARY_OP_DISPATCH_IMPL
#undef ___PRO_EXTENDED_BINARY_OP_DISPATCH_IMPL
#undef ___PRO_RHS_OP_DISPATCH_IMPL
#undef ___PRO_DEF_RHS_OP_ACCESSOR
#undef ___PRO_LHS_OP_DISPATCH_IMPL
#undef ___PRO_LHS_ALL_OP_DISPATCH_BODY_IMPL
#undef ___PRO_LHS_BINARY_OP_DISPATCH_BODY_IMPL
#undef ___PRO_LHS_UNARY_OP_DISPATCH_BODY_IMPL
#undef ___PRO_LHS_LEFT_OP_DISPATCH_BODY_IMPL
#undef ___PRO_DEF_LHS_ALL_OP_ACCESSOR
#undef ___PRO_DEF_LHS_BINARY_OP_ACCESSOR
#undef ___PRO_DEF_LHS_UNARY_OP_ACCESSOR
#undef ___PRO_DEF_LHS_ANY_OP_ACCESSOR
#undef ___PRO_DEF_LHS_LEFT_OP_ACCESSOR

struct implicit_conversion_dispatch
    : details::cast_dispatch_base<false, false> {
  template <class T>
  ___PRO3_STATIC_CALL(T&&, T&& self) noexcept { return std::forward<T>(self); }
};
struct explicit_conversion_dispatch : details::cast_dispatch_base<true, false> {
  template <class T>
  ___PRO3_STATIC_CALL(auto, T&& self) noexcept
      { return details::explicit_conversion_adapter<T>{std::forward<T>(self)}; }
};
using conversion_dispatch = explicit_conversion_dispatch;

class not_implemented : public std::exception {
 public:
  char const* what() const noexcept override
      { return "pro::v3::not_implemented"; }
};

template <class D>
struct weak_dispatch : D {
  using D::operator();
  template <class... Args>
  [[noreturn]] ___PRO3_STATIC_CALL(details::wildcard, std::nullptr_t, Args&&...)
      { ___PRO_THROW(not_implemented{}); }
};

#define PRO_DEF_WEAK_DISPATCH(__NAME, __D, __FUNC) \
    struct [[deprecated("'PRO_DEF_WEAK_DISPATCH' is deprecated. " \
        "Use pro::weak_dispatch<" #__D "> instead.")]] __NAME : __D { \
      using __D::operator(); \
      template <class... __Args> \
      ___PRO3_STATIC_CALL(decltype(auto), \
          ::std::nullptr_t, __Args&&... __args) \
          ___PRO3_DIRECT_FUNC_IMPL(__FUNC(::std::forward<__Args>(__args)...)) \
    }

}  // namespace pro

#if __STDC_HOSTED__
namespace std {

template <pro::v3::facade F, class CharT>
    requires(pro::v3::details::facade_traits<F>::template is_invocable<
        false, pro::v3::details::format_dispatch,
        pro::v3::details::format_overload_t<CharT>>)
struct formatter<pro::v3::proxy_indirect_accessor<F>, CharT> {
  constexpr auto parse(basic_format_parse_context<CharT>& pc) {
    for (auto it = pc.begin(); it != pc.end(); ++it) {
      if (*it == '}') {
        spec_ = basic_string_view<CharT>{pc.begin(), it + 1};
        return it;
      }
    }
    return pc.end();
  }

  template <class OutIt>
  OutIt format(const pro::v3::proxy_indirect_accessor<F>& ia,
      basic_format_context<OutIt, CharT>& fc) const {
    auto& p = pro::v3::access_proxy<F>(ia);
    if (!p.has_value()) { ___PRO_THROW(format_error{"null proxy"}); }
    return pro::v3::proxy_invoke<false, pro::details::format_dispatch,
        pro::v3::details::format_overload_t<CharT>>(p, spec_, fc);
  }

 private:
  basic_string_view<CharT> spec_;
};

}  // namespace std
#endif  // __STDC_HOSTED__

// Version-less macro aliases

#define ___PRO3_AMBIGUOUS_MACRO_DIAGNOSTIC_ASSERT(__NAME,                      \
                                                  __VERSION_QUALIFIED_NAME)    \
  static_assert(false, "The use of macro `" #__NAME "` is ambiguous. \
Are multiple different versions of Proxy library included at the same time?\n\
Note: To resolve this error: \n\
- Either make sure that only one version of Proxy library is included within this file.\n\
- Or use the `" #__VERSION_QUALIFIED_NAME                                      \
                       "` macro (note the `3` suffix) to explicitly \
stick to a specific major version of the Proxy library.")

#ifdef __msft_lib_proxy
#undef __msft_lib_proxy
#define __msft_lib_proxy                                                       \
  [] {                                                                         \
    ___PRO3_AMBIGUOUS_MACRO_DIAGNOSTIC_ASSERT(__msft_lib_proxy,                \
                                              __msft_lib_proxy3);              \
    return 0L;                                                                 \
  }()
#else
#define __msft_lib_proxy __msft_lib_proxy3
#endif // __msft_lib_proxy

#ifdef PRO_DEF_MEM_DISPATCH
#undef PRO_DEF_MEM_DISPATCH
#define PRO_DEF_MEM_DISPATCH(...)                                              \
  ___PRO3_AMBIGUOUS_MACRO_DIAGNOSTIC_ASSERT(PRO_DEF_MEM_DISPATCH,              \
                                            PRO3_DEF_MEM_DISPATCH)
#else
#define PRO_DEF_MEM_DISPATCH(name, ...) PRO3_DEF_MEM_DISPATCH(name, __VA_ARGS__)
#endif // PRO_DEF_MEM_DISPATCH

#ifdef PRO_DEF_FREE_DISPATCH
#undef PRO_DEF_FREE_DISPATCH
#define PRO_DEF_FREE_DISPATCH(...)                                             \
  ___PRO3_AMBIGUOUS_MACRO_DIAGNOSTIC_ASSERT(PRO_DEF_FREE_DISPATCH,             \
                                            PRO3_DEF_FREE_DISPATCH)
#else
#define PRO_DEF_FREE_DISPATCH(name, ...)                                       \
  PRO3_DEF_FREE_DISPATCH(name, __VA_ARGS__)
#endif // PRO_DEF_FREE_DISPATCH

#ifdef PRO_DEF_FREE_AS_MEM_DISPATCH
#undef PRO_DEF_FREE_AS_MEM_DISPATCH
#define PRO_DEF_FREE_AS_MEM_DISPATCH(...)                                      \
  ___PRO3_AMBIGUOUS_MACRO_DIAGNOSTIC_ASSERT(PRO_DEF_FREE_AS_MEM_DISPATCH,      \
                                            PRO3_DEF_FREE_AS_MEM_DISPATCH)
#else
#define PRO_DEF_FREE_AS_MEM_DISPATCH(name, ...)                                \
  PRO3_DEF_FREE_AS_MEM_DISPATCH(name, __VA_ARGS__)
#endif // PRO_DEF_FREE_AS_MEM_DISPATCH

#undef ___PRO_THROW
#undef ___PRO_NO_UNIQUE_ADDRESS_ATTRIBUTE

#endif  // _MSFT_PROXY3_
