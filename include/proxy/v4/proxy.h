// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#ifndef MSFT_PROXY_V4_PROXY_H_
#define MSFT_PROXY_V4_PROXY_H_

#include <bit>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdlib>
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
#if __has_include(<format>)
#include <format>
#endif // __has_include(<format>)
#if __cpp_lib_format >= 201907L ||                                             \
    (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 170000)
#define PRO4D_HAS_FORMAT
#endif // __cpp_lib_format || _LIBCPP_VERSION >= 170000
#endif // __STDC_HOSTED__

#if __cpp_rtti >= 199711L
#include <optional>
#include <typeinfo>
#endif // __cpp_rtti >= 199711L

#include "proxy_macros.h"

#if __has_cpp_attribute(msvc::no_unique_address)
#define PROD_NO_UNIQUE_ADDRESS_ATTRIBUTE msvc::no_unique_address
#elif __has_cpp_attribute(no_unique_address)
#define PROD_NO_UNIQUE_ADDRESS_ATTRIBUTE no_unique_address
#else
#error Proxy requires C++20 attribute no_unique_address.
#endif

#if __cpp_lib_unreachable >= 202202L
#define PROD_UNREACHABLE() std::unreachable()
#else
#define PROD_UNREACHABLE() std::abort()
#endif // __cpp_lib_unreachable >= 202202L

namespace pro::inline v4 {

// =============================================================================
// == Core Components (facade, proxy, etc.)                                   ==
// =============================================================================

namespace details {

template <class F>
struct basic_facade_traits;

} // namespace details

enum class constraint_level { none, nontrivial, nothrow, trivial };

template <template <class> class O>
struct facade_aware_overload_t {
  facade_aware_overload_t() = delete;
};

template <class F>
concept facade = details::basic_facade_traits<F>::applicable;

template <facade F>
class proxy_indirect_accessor;
template <facade F>
class PRO4D_ENFORCE_EBO proxy;

template <class T>
struct is_bitwise_trivially_relocatable
    : std::bool_constant<std::is_trivially_move_constructible_v<T> &&
                         std::is_trivially_destructible_v<T>> {};
template <class T>
constexpr bool is_bitwise_trivially_relocatable_v =
    is_bitwise_trivially_relocatable<T>::value;

namespace details {

struct applicable_traits {
  static constexpr bool applicable = true;
};
struct inapplicable_traits {
  static constexpr bool applicable = false;
};

template <template <class, class> class R, class O, class... Is>
struct recursive_reduction : std::type_identity<O> {};
template <template <class, class> class R, class O, class I, class... Is>
struct recursive_reduction<R, O, I, Is...>
    : recursive_reduction<R, R<O, I>, Is...> {};
template <template <class, class> class R, class O, class... Is>
using recursive_reduction_t = typename recursive_reduction<R, O, Is...>::type;

template <template <class...> class R, class... Args>
struct reduction_traits {
  template <class O, class I>
  using type = typename R<Args..., O, I>::type;
};

template <class O, class I>
struct composition_reduction : std::type_identity<O> {};
template <template <class...> class T, class... Os, class I>
  requires(!std::is_void_v<I>)
struct composition_reduction<T<Os...>, I> : std::type_identity<T<Os..., I>> {};
template <template <class...> class T, class... Os, class... Is>
struct composition_reduction<T<Os...>, T<Is...>>
    : std::type_identity<T<Os..., Is...>> {};
template <class T, class... Us>
using composite_t = recursive_reduction_t<
    reduction_traits<composition_reduction>::template type, T, Us...>;

template <class Expr>
consteval bool is_consteval(Expr) {
  return requires { typename std::bool_constant<(Expr{}(), false)>; };
}
template <class T, class U>
concept static_prop = std::is_same_v<T, const U&>;

template <class T, std::size_t I>
concept has_tuple_element = requires { typename std::tuple_element_t<I, T>; };
template <class T>
consteval bool is_tuple_like_well_formed() {
  if constexpr (requires {
                  { std::tuple_size<T>::value } -> static_prop<std::size_t>;
                }) {
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

enum class qualifier_type { lv, const_lv, rv, const_rv };
template <class T, qualifier_type Q>
struct add_qualifier_traits;
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

template <class T, constraint_level CL>
struct copyability_traits : inapplicable_traits {};
template <class T>
struct copyability_traits<T, constraint_level::none> : applicable_traits {};
template <class T>
  requires(std::is_copy_constructible_v<T>)
struct copyability_traits<T, constraint_level::nontrivial> : applicable_traits {
};
template <class T>
  requires(std::is_nothrow_copy_constructible_v<T>)
struct copyability_traits<T, constraint_level::nothrow> : applicable_traits {};
template <class T>
  requires(std::is_trivially_copy_constructible_v<T>)
struct copyability_traits<T, constraint_level::trivial> : applicable_traits {};

template <class T, constraint_level CL>
struct relocatability_traits : inapplicable_traits {};
template <class T>
struct relocatability_traits<T, constraint_level::none> : applicable_traits {};
template <class T>
  requires((std::is_move_constructible_v<T> && std::is_destructible_v<T>) ||
           is_bitwise_trivially_relocatable_v<T>)
struct relocatability_traits<T, constraint_level::nontrivial>
    : applicable_traits {};
template <class T>
  requires((std::is_nothrow_move_constructible_v<T> &&
            std::is_nothrow_destructible_v<T>) ||
           is_bitwise_trivially_relocatable_v<T>)
struct relocatability_traits<T, constraint_level::nothrow> : applicable_traits {
};
template <class T>
  requires(is_bitwise_trivially_relocatable_v<T>)
struct relocatability_traits<T, constraint_level::trivial> : applicable_traits {
};

template <class T, constraint_level CL>
struct destructibility_traits : inapplicable_traits {};
template <class T>
struct destructibility_traits<T, constraint_level::none> : applicable_traits {};
template <class T>
  requires(std::is_destructible_v<T>)
struct destructibility_traits<T, constraint_level::nontrivial>
    : applicable_traits {};
template <class T>
  requires(std::is_nothrow_destructible_v<T>)
struct destructibility_traits<T, constraint_level::nothrow>
    : applicable_traits {};
template <class T>
  requires(std::is_trivially_destructible_v<T>)
struct destructibility_traits<T, constraint_level::trivial>
    : applicable_traits {};

struct proxy_helper {
  template <class P, class F>
  struct resetting_guard {
    explicit resetting_guard(proxy<F>& p) noexcept : p_(p) {}
    ~resetting_guard() noexcept(std::is_nothrow_destructible_v<P>) {
      std::destroy_at(std::addressof(get_ptr<P, F, qualifier_type::lv>(p_)));
      p_.meta_.reset();
    }

  private:
    proxy<F>& p_;
  };

  template <class F>
  static const auto& get_meta(const proxy<F>& p) noexcept {
    assert(p.has_value());
    return *p.meta_.operator->();
  }
  template <class P, class F, qualifier_type Q>
  static add_qualifier_t<P, Q> get_ptr(add_qualifier_t<proxy<F>, Q> p) {
    return static_cast<add_qualifier_t<P, Q>>(
        *std::launder(reinterpret_cast<add_qualifier_ptr_t<P, Q>>(p.ptr_)));
  }
  template <class P, class F1, class F2>
  static void trivially_relocate(proxy<F1>& from, proxy<F2>& to) noexcept {
    std::uninitialized_copy_n(from.ptr_, sizeof(P), to.ptr_);
    to.meta_ = decltype(proxy<F2>::meta_){std::in_place_type<P>};
    from.meta_.reset();
  }
};

template <class P, bool IsDirect, qualifier_type Q>
struct operand_traits : add_qualifier_traits<P, Q> {};
template <class P, qualifier_type Q>
struct operand_traits<P, false, Q>
    : std::type_identity<decltype(*std::declval<add_qualifier_t<P, Q>>())> {};
template <class P, bool IsDirect, qualifier_type Q>
using operand_t = typename operand_traits<P, IsDirect, Q>::type;
template <class P, bool IsDirect, class D, qualifier_type Q, bool NE, class R,
          class... Args>
concept invocable_dispatch =
    (IsDirect || (requires { *std::declval<add_qualifier_t<P, Q>>(); } &&
                  (!NE || noexcept(*std::declval<add_qualifier_t<P, Q>>())))) &&
    ((NE && std::is_nothrow_invocable_r_v<R, D, operand_t<P, IsDirect, Q>,
                                          Args...>) ||
     (!NE &&
      std::is_invocable_r_v<R, D, operand_t<P, IsDirect, Q>, Args...>)) &&
    (Q != qualifier_type::rv || (NE && std::is_nothrow_destructible_v<P>) ||
     (!NE && std::is_destructible_v<P>));

template <class D, class R, class... Args>
R invoke_dispatch_impl(Args&&... args) {
  if constexpr (std::is_void_v<R>) {
    D()(std::forward<Args>(args)...);
  } else {
    return D()(std::forward<Args>(args)...);
  }
}
template <bool IsDirect, class P>
decltype(auto) get_operand(P&& ptr) {
  if constexpr (IsDirect) {
    return std::forward<P>(ptr);
  } else {
    if constexpr (std::is_constructible_v<bool, P&>) {
      assert(ptr);
    }
    return *std::forward<P>(ptr);
  }
}
struct internal_dispatch {};
template <class P, class F, bool IsDirect, class D, qualifier_type Q, bool NE,
          class R, class... Args>
R invoke_dispatch(add_qualifier_t<proxy<F>, Q> self,
                  Args... args) noexcept(NE) {
  if constexpr (Q == qualifier_type::rv) {
    if constexpr (std::is_base_of_v<internal_dispatch, D> &&
                  is_bitwise_trivially_relocatable_v<P>) {
      static_assert(IsDirect);
      return D{}(std::in_place_type<P>,
                 std::forward<add_qualifier_t<proxy<F>, Q>>(self),
                 std::forward<Args>(args)...);
    } else {
      proxy_helper::resetting_guard<P, F> guard{self};
      return invoke_dispatch_impl<D, R>(
          get_operand<IsDirect>(
              proxy_helper::get_ptr<P, F, Q>(std::move(self))),
          std::forward<Args>(args)...);
    }
  } else {
    return invoke_dispatch_impl<D, R>(
        get_operand<IsDirect>(proxy_helper::get_ptr<P, F, Q>(
            std::forward<add_qualifier_t<proxy<F>, Q>>(self))),
        std::forward<Args>(args)...);
  }
}

template <class O>
struct overload_traits : inapplicable_traits {};
template <qualifier_type Q, bool NE, class R, class... Args>
struct overload_traits_impl : applicable_traits {
  using return_type = R;
  template <class F>
  using dispatcher_type = R (*)(add_qualifier_t<proxy<F>, Q>,
                                Args...) noexcept(NE);

  template <class P, class F, bool IsDirect, class D>
  static constexpr auto dispatcher =
      &invoke_dispatch<P, F, IsDirect, D, Q, NE, R, Args...>;

  template <class P, bool IsDirect, class D>
  static constexpr bool applicable_ptr =
      invocable_dispatch<P, IsDirect, D, Q, NE, R, Args...>;
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
struct overload_traits<R(Args...) const & noexcept>
    : overload_traits_impl<qualifier_type::const_lv, true, R, Args...> {};
template <class R, class... Args>
struct overload_traits<R(Args...) const&&>
    : overload_traits_impl<qualifier_type::const_rv, false, R, Args...> {};
template <class R, class... Args>
struct overload_traits<R(Args...) const && noexcept>
    : overload_traits_impl<qualifier_type::const_rv, true, R, Args...> {};

template <class O>
struct overload_substitution_traits : inapplicable_traits {
  template <class>
  using type = O;
};
template <template <class> class O>
struct overload_substitution_traits<facade_aware_overload_t<O>>
    : applicable_traits {
  template <class F>
  using type = O<F>;
};
template <class O, class F>
using substituted_overload_t =
    typename overload_substitution_traits<O>::template type<F>;
template <class O>
concept extended_overload = overload_traits<O>::applicable ||
                            overload_substitution_traits<O>::applicable;
template <class P, class F, bool IsDirect, class D, class O>
consteval bool diagnose_proxiable_required_convention_not_implemented() {
  constexpr bool verdict =
      overload_traits<O>::applicable &&
      overload_traits<O>::template applicable_ptr<P, IsDirect, D>;
  static_assert(verdict,
                "not proxiable due to a required convention not implemented");
  return verdict;
}

template <class F, bool IsDirect, class D, class O>
struct invocation_meta {
  invocation_meta() = default;
  template <class P>
  constexpr explicit invocation_meta(std::in_place_type_t<P>)
      : dispatcher(overload_traits<O>::template dispatcher<P, F, IsDirect, D>) {
  }

  typename overload_traits<O>::template dispatcher_type<F> dispatcher;
};

template <class... Ms>
struct PRO4D_ENFORCE_EBO composite_meta : Ms... {
  composite_meta() = default;
  template <class P>
  constexpr explicit composite_meta(std::in_place_type_t<P>)
      : Ms(std::in_place_type<P>)... {}
};

template <class T>
consteval bool is_is_direct_well_formed() {
  if constexpr (requires {
                  { T::is_direct } -> static_prop<bool>;
                }) {
    if constexpr (is_consteval([] { return T::is_direct; })) {
      return true;
    }
  }
  return false;
}

template <class C, class... Os>
struct basic_conv_traits_impl : inapplicable_traits {};
template <class C, extended_overload... Os>
  requires(sizeof...(Os) > 0u)
struct basic_conv_traits_impl<C, Os...> : applicable_traits {};
template <class C>
struct basic_conv_traits : inapplicable_traits {};
template <class C>
  requires(requires {
    { typename C::dispatch_type() } noexcept;
    typename C::overload_types;
  } && is_is_direct_well_formed<C>() &&
           is_tuple_like_well_formed<typename C::overload_types>())
struct basic_conv_traits<C>
    : instantiated_t<basic_conv_traits_impl, typename C::overload_types, C> {};

template <class T>
struct a11y_traits_impl
    : std::conditional<std::is_nothrow_default_constructible_v<T> &&
                           std::is_trivially_copyable_v<T> &&
                           !std::is_final_v<T>,
                       T, void> {};
template <class SFINAE, class T, class... Args>
struct a11y_traits : std::type_identity<void> {};
template <class T, class... Args>
struct a11y_traits<std::void_t<typename T::template accessor<Args...>>, T,
                   Args...>
    : a11y_traits_impl<typename T::template accessor<Args...>> {};
template <class T, class... Args>
using accessor_t = typename a11y_traits<void, T, Args...>::type;

template <class C, class F, class... Os>
struct conv_traits_impl : inapplicable_traits {};
template <class C, class F, class... Os>
  requires(overload_traits<substituted_overload_t<Os, F>>::applicable && ...)
struct conv_traits_impl<C, F, Os...> : applicable_traits {
  using meta =
      composite_meta<invocation_meta<F, C::is_direct, typename C::dispatch_type,
                                     substituted_overload_t<Os, F>>...>;
  template <class T>
  using accessor =
      accessor_t<typename C::dispatch_type, T, typename C::dispatch_type,
                 substituted_overload_t<Os, F>...>;

  template <class P>
  static consteval bool diagnose_proxiable() {
    bool verdict = true;
    ((verdict &= diagnose_proxiable_required_convention_not_implemented<
          P, F, C::is_direct, typename C::dispatch_type,
          substituted_overload_t<Os, F>>()),
     ...);
    return verdict;
  }

  template <class P>
  static constexpr bool applicable_ptr =
      (overload_traits<substituted_overload_t<Os, F>>::template applicable_ptr<
           P, C::is_direct, typename C::dispatch_type> &&
       ...);
};
template <class C, class F>
struct conv_traits
    : instantiated_t<conv_traits_impl, typename C::overload_types, C, F> {};

template <bool IsDirect, class R>
struct refl_meta {
  refl_meta() = default;
  template <class P>
    requires(IsDirect)
  constexpr explicit refl_meta(std::in_place_type_t<P>)
      : reflector(std::in_place_type<P>) {}
  template <class P>
    requires(!IsDirect)
  constexpr explicit refl_meta(std::in_place_type_t<P>)
      : reflector(
            std::in_place_type<typename std::pointer_traits<P>::element_type>) {
  }

  [[PROD_NO_UNIQUE_ADDRESS_ATTRIBUTE]]
  R reflector;
};

template <class R>
struct basic_refl_traits : inapplicable_traits {};
template <class R>
  requires(requires { typename R::reflector_type; } &&
           is_is_direct_well_formed<R>())
struct basic_refl_traits<R> : applicable_traits {};

template <class T, bool IsDirect, class R>
consteval bool is_reflector_well_formed() {
  if constexpr (IsDirect) {
    if constexpr (std::is_constructible_v<R, std::in_place_type_t<T>>) {
      if constexpr (is_consteval([] { return R(std::in_place_type<T>); })) {
        return true;
      }
    }
  } else {
    return is_reflector_well_formed<
        typename std::pointer_traits<T>::element_type, true, R>();
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
  PRO4D_STATIC_CALL(void, const T& self, proxy<F>& rhs) noexcept(
      std::is_nothrow_copy_constructible_v<T>) {
    std::construct_at(std::addressof(rhs), self);
  }
};
struct relocate_dispatch : internal_dispatch {
  template <class P, class F>
  PRO4D_STATIC_CALL(void, std::in_place_type_t<P>, proxy<F>&& self,
                    proxy<F>& rhs) noexcept {
    proxy_helper::trivially_relocate<P>(self, rhs);
  }
  template <class T, class F>
  PRO4D_STATIC_CALL(void, T&& self, proxy<F>& rhs) noexcept(
      relocatability_traits<T, constraint_level::nothrow>::applicable) {
    std::construct_at(std::addressof(rhs), std::forward<T>(self));
  }
};
struct destroy_dispatch {
  template <class T>
  PRO4D_STATIC_CALL(void, T& self) noexcept(std::is_nothrow_destructible_v<T>) {
    std::destroy_at(&self);
  }
};
template <class F, class D, class ONE, class OE, constraint_level C>
struct lifetime_meta_traits : std::type_identity<void> {};
template <class F, class D, class ONE, class OE>
struct lifetime_meta_traits<F, D, ONE, OE, constraint_level::nothrow>
    : std::type_identity<invocation_meta<F, true, D, ONE>> {};
template <class F, class D, class ONE, class OE>
struct lifetime_meta_traits<F, D, ONE, OE, constraint_level::nontrivial>
    : std::type_identity<invocation_meta<F, true, D, OE>> {};
template <class F, class D, class ONE, class OE, constraint_level C>
using lifetime_meta_t = typename lifetime_meta_traits<F, D, ONE, OE, C>::type;

template <class... As>
struct PRO4D_ENFORCE_EBO composite_accessor : As... {};

template <class C, class F, bool IsDirect>
struct conv_accessor_traits : std::type_identity<void> {};
template <class C, class F>
  requires(!C::is_direct)
struct conv_accessor_traits<C, F, false>
    : std::type_identity<typename conv_traits<C, F>::template accessor<
          proxy_indirect_accessor<F>>> {};
template <class C, class F>
  requires(C::is_direct)
struct conv_accessor_traits<C, F, true>
    : std::type_identity<
          typename conv_traits<C, F>::template accessor<proxy<F>>> {};
template <class C, class F, bool IsDirect>
using conv_accessor_t = typename conv_accessor_traits<C, F, IsDirect>::type;

template <class R, class F, bool IsDirect>
struct refl_accessor_traits : std::type_identity<void> {};
template <class R, class F>
  requires(!R::is_direct)
struct refl_accessor_traits<R, F, false>
    : std::type_identity<
          accessor_t<typename R::reflector_type, proxy_indirect_accessor<F>,
                     typename R::reflector_type>> {};
template <class R, class F>
  requires(R::is_direct)
struct refl_accessor_traits<R, F, true>
    : std::type_identity<accessor_t<typename R::reflector_type, proxy<F>,
                                    typename R::reflector_type>> {};
template <class R, class F, bool IsDirect>
using refl_accessor_t = typename refl_accessor_traits<R, F, IsDirect>::type;

template <class P>
struct ptr_traits : inapplicable_traits {};
template <class P>
  requires((
               requires { *std::declval<P&>(); } ||
               requires { typename P::element_type; }) &&
           requires { typename std::pointer_traits<P>::element_type; })
struct ptr_traits<P> : applicable_traits {};
template <class F>
struct ptr_traits<proxy<F>> : inapplicable_traits {};

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
  constexpr bool verdict =
      copyability_traits<P, RequiredCopyability>::applicable;
  static_assert(verdict, "not proxiable due to insufficient copyability");
  return verdict;
}
template <class P, class F, constraint_level RequiredRelocatability>
consteval bool diagnose_proxiable_insufficient_relocatability() {
  constexpr bool verdict =
      relocatability_traits<P, RequiredRelocatability>::applicable;
  static_assert(verdict, "not proxiable due to insufficient relocatability");
  return verdict;
}
template <class P, class F, constraint_level RequiredDestructibility>
consteval bool diagnose_proxiable_insufficient_destructibility() {
  constexpr bool verdict =
      destructibility_traits<P, RequiredDestructibility>::applicable;
  static_assert(verdict, "not proxiable due to insufficient destructibility");
  return verdict;
}

consteval bool is_layout_well_formed(std::size_t size, std::size_t align) {
  return size > 0u && std::has_single_bit(align) && size % align == 0u;
}
consteval bool is_cl_well_formed(constraint_level cl) {
  return cl >= constraint_level::none && cl <= constraint_level::trivial;
}
template <class F>
consteval bool is_facade_constraints_well_formed() {
  if constexpr (requires {
                  { F::max_size } -> static_prop<std::size_t>;
                  { F::max_align } -> static_prop<std::size_t>;
                  { F::copyability } -> static_prop<constraint_level>;
                  { F::relocatability } -> static_prop<constraint_level>;
                  { F::destructibility } -> static_prop<constraint_level>;
                }) {
    if constexpr (is_consteval([] {
                    return std::tuple{F::max_size, F::max_align, F::copyability,
                                      F::relocatability, F::destructibility};
                  })) {
      return is_layout_well_formed(F::max_size, F::max_align) &&
             is_cl_well_formed(F::copyability) &&
             is_cl_well_formed(F::relocatability) &&
             is_cl_well_formed(F::destructibility);
    }
  }
  return false;
}
template <class... Cs>
struct basic_facade_conv_traits_impl : inapplicable_traits {};
template <class... Cs>
  requires(basic_conv_traits<Cs>::applicable && ...)
struct basic_facade_conv_traits_impl<Cs...> : applicable_traits {};
template <class... Rs>
struct basic_facade_refl_traits_impl : inapplicable_traits {};
template <class... Rs>
  requires(basic_refl_traits<Rs>::applicable && ...)
struct basic_facade_refl_traits_impl<Rs...> : applicable_traits {};
template <class F>
struct basic_facade_traits : inapplicable_traits {};
template <class F>
  requires(requires {
    typename F::convention_types;
    typename F::reflection_types;
  } && is_facade_constraints_well_formed<F>() &&
           is_tuple_like_well_formed<typename F::convention_types>() &&
           instantiated_t<basic_facade_conv_traits_impl,
                          typename F::convention_types>::applicable &&
           is_tuple_like_well_formed<typename F::reflection_types>() &&
           instantiated_t<basic_facade_refl_traits_impl,
                          typename F::reflection_types>::applicable)
struct basic_facade_traits<F> : applicable_traits {};

template <class F, class... Cs>
struct facade_conv_traits_impl : inapplicable_traits {};
template <class F, class... Cs>
  requires(conv_traits<Cs, F>::applicable && ...)
struct facade_conv_traits_impl<F, Cs...> : applicable_traits {
  using conv_meta =
      composite_t<composite_meta<>, typename conv_traits<Cs, F>::meta...>;
  using conv_indirect_accessor =
      composite_t<composite_accessor<>, conv_accessor_t<Cs, F, false>...>;
  using conv_direct_accessor =
      composite_t<composite_accessor<>, conv_accessor_t<Cs, F, true>...>;

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
  static constexpr bool is_invocable =
      std::is_base_of_v<invocation_meta<F, IsDirect, D, O>, conv_meta>;
};
template <class F, class... Rs>
struct facade_refl_traits_impl {
  using refl_meta = composite_meta<typename refl_traits<Rs>::meta...>;
  using refl_indirect_accessor =
      composite_t<composite_accessor<>, refl_accessor_t<Rs, F, false>...>;
  using refl_direct_accessor =
      composite_t<composite_accessor<>, refl_accessor_t<Rs, F, true>...>;

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
template <class F>
struct facade_traits : inapplicable_traits {};
template <class F>
  requires(instantiated_t<facade_conv_traits_impl, typename F::convention_types,
                          F>::applicable)
struct facade_traits<F>
    : instantiated_t<facade_conv_traits_impl, typename F::convention_types, F>,
      instantiated_t<facade_refl_traits_impl, typename F::reflection_types, F> {
  using meta = composite_t<
      composite_meta<>,
      lifetime_meta_t<F, copy_dispatch, void(proxy<F>&) const noexcept,
                      void(proxy<F>&) const, F::copyability>,
      lifetime_meta_t<F, relocate_dispatch, void(proxy<F>&) && noexcept,
                      void(proxy<F>&) &&, F::relocatability>,
      lifetime_meta_t<F, destroy_dispatch, void() noexcept, void(),
                      F::destructibility>,
      typename facade_traits::conv_meta, typename facade_traits::refl_meta>;
  using indirect_accessor =
      composite_t<typename facade_traits::conv_indirect_accessor,
                  typename facade_traits::refl_indirect_accessor>;
  using direct_accessor =
      composite_t<typename facade_traits::conv_direct_accessor,
                  typename facade_traits::refl_direct_accessor>;

  template <class P>
  static consteval void diagnose_proxiable() {
    bool verdict = true;
    verdict &=
        diagnose_proxiable_size_too_large<P, F, sizeof(P), F::max_size>();
    verdict &=
        diagnose_proxiable_align_too_large<P, F, alignof(P), F::max_align>();
    verdict &=
        diagnose_proxiable_insufficient_copyability<P, F, F::copyability>();
    verdict &=
        diagnose_proxiable_insufficient_relocatability<P, F,
                                                       F::relocatability>();
    verdict &=
        diagnose_proxiable_insufficient_destructibility<P, F,
                                                        F::destructibility>();
    verdict &= facade_traits::template diagnose_proxiable_conv<P>();
    verdict &= facade_traits::template diagnose_proxiable_refl<P>();
    if (!verdict) {
      PROD_UNREACHABLE(); // Propagate the error to the caller side
    }
  }

  template <class P>
  static constexpr bool applicable_ptr =
      sizeof(P) <= F::max_size && alignof(P) <= F::max_align &&
      copyability_traits<P, F::copyability>::applicable &&
      relocatability_traits<P, F::relocatability>::applicable &&
      destructibility_traits<P, F::destructibility>::applicable &&
      facade_traits::template conv_applicable_ptr<P> &&
      facade_traits::template refl_applicable_ptr<P>;
};

using ptr_prototype = void* [2];

template <class M>
struct meta_ptr_indirect_impl {
  meta_ptr_indirect_impl() = default;
  template <class P>
  explicit meta_ptr_indirect_impl(std::in_place_type_t<P>)
      : ptr_(&storage<P>) {}
  bool has_value() const noexcept { return ptr_ != nullptr; }
  void reset() noexcept { ptr_ = nullptr; }
  const M* operator->() const noexcept { return ptr_; }

private:
  const M* ptr_;
  template <class P>
  static constexpr M storage{std::in_place_type<P>};
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
template <class F, bool IsDirect, class D, class O, class... Ms>
struct meta_ptr_traits_impl<
    composite_meta<invocation_meta<F, IsDirect, D, O>, Ms...>>
    : std::type_identity<meta_ptr_direct_impl<
          composite_meta<invocation_meta<F, IsDirect, D, O>, Ms...>,
          invocation_meta<F, IsDirect, D, O>>> {};
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

template <class T>
class inplace_ptr {
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
  [[PROD_NO_UNIQUE_ADDRESS_ATTRIBUTE]]
  T value_;
};

template <class F, bool IsDirect, class D, class O, class P, class... Args>
decltype(auto) invoke_impl(P&& p, Args&&... args) {
  auto dispatcher =
      proxy_helper::get_meta(p)
          .template invocation_meta<F, IsDirect, D, O>::dispatcher;
  return dispatcher(std::forward<P>(p), std::forward<Args>(args)...);
}
template <class F, qualifier_type Q>
add_qualifier_t<proxy<F>, Q>
    as_proxy(add_qualifier_t<proxy_indirect_accessor<F>, Q> p) {
  return static_cast<add_qualifier_t<proxy<F>, Q>>(
      *reinterpret_cast<
          add_qualifier_ptr_t<inplace_ptr<proxy_indirect_accessor<F>>, Q>>(
          std::addressof(p)));
}

} // namespace details

template <class P, class F>
concept proxiable = facade<F> && details::facade_traits<F>::applicable &&
                    details::ptr_traits<P>::applicable &&
                    details::facade_traits<F>::template applicable_ptr<P>;

template <facade F>
class proxy_indirect_accessor
    : public details::facade_traits<F>::indirect_accessor {
  friend class details::inplace_ptr<proxy_indirect_accessor>;
  proxy_indirect_accessor() = default;
  proxy_indirect_accessor(const proxy_indirect_accessor&) = default;
  proxy_indirect_accessor& operator=(const proxy_indirect_accessor&) = default;
};

template <facade F>
class proxy : public details::facade_traits<F>::direct_accessor,
              public details::inplace_ptr<proxy_indirect_accessor<F>> {
  friend struct details::proxy_helper;
  static_assert(details::facade_traits<F>::applicable);

public:
  using facade_type = F;

  proxy() noexcept { initialize(); }
  proxy(std::nullptr_t) noexcept : proxy() {}
  proxy(const proxy&) noexcept
    requires(F::copyability == constraint_level::trivial)
  = default;
  proxy(const proxy& rhs) noexcept(F::copyability == constraint_level::nothrow)
    requires(F::copyability == constraint_level::nontrivial ||
             F::copyability == constraint_level::nothrow)
      : details::inplace_ptr<
            proxy_indirect_accessor<F>>() /* Make GCC happy */ {
    initialize(rhs);
  }
  proxy(proxy&& rhs) noexcept(F::relocatability >= constraint_level::nothrow)
    requires(F::relocatability >= constraint_level::nontrivial &&
             F::copyability != constraint_level::trivial)
  {
    initialize(std::move(rhs));
  }
  template <class P>
  constexpr proxy(P&& ptr) noexcept(
      std::is_nothrow_constructible_v<std::decay_t<P>, P>)
    requires(details::ptr_traits<std::decay_t<P>>::applicable &&
             std::is_constructible_v<std::decay_t<P>, P>)
  {
    initialize<std::decay_t<P>>(std::forward<P>(ptr));
  }
  template <class P, class... Args>
  constexpr explicit proxy(std::in_place_type_t<P>, Args&&... args) noexcept(
      std::is_nothrow_constructible_v<P, Args...>)
    requires(details::ptr_traits<P>::applicable &&
             std::is_constructible_v<P, Args...>)
  {
    initialize<P>(std::forward<Args>(args)...);
  }
  template <class P, class U, class... Args>
  constexpr explicit proxy(
      std::in_place_type_t<P>, std::initializer_list<U> il,
      Args&&... args) noexcept(std::
                                   is_nothrow_constructible_v<
                                       P, std::initializer_list<U>&, Args...>)
    requires(details::ptr_traits<P>::applicable &&
             std::is_constructible_v<P, std::initializer_list<U>&, Args...>)
  {
    initialize<P>(il, std::forward<Args>(args)...);
  }
  proxy& operator=(std::nullptr_t) noexcept(F::destructibility >=
                                            constraint_level::nothrow)
    requires(F::destructibility >= constraint_level::nontrivial)
  {
    reset();
    return *this;
  }
  proxy& operator=(const proxy&) noexcept
    requires(F::copyability == constraint_level::trivial)
  = default;
  proxy& operator=(const proxy& rhs) noexcept(F::copyability >=
                                                  constraint_level::nothrow &&
                                              F::destructibility >=
                                                  constraint_level::nothrow)
    requires((F::copyability == constraint_level::nontrivial ||
              F::copyability == constraint_level::nothrow) &&
             F::destructibility >= constraint_level::nontrivial)
  {
    if (this != std::addressof(rhs)) [[likely]] {
      if constexpr (F::copyability == constraint_level::nothrow) {
        destroy();
        initialize(rhs);
      } else {
        *this = proxy{rhs};
      }
    }
    return *this;
  }
  proxy& operator=(proxy&& rhs) noexcept(F::relocatability >=
                                             constraint_level::nothrow &&
                                         F::destructibility >=
                                             constraint_level::nothrow)
    requires(F::relocatability >= constraint_level::nontrivial &&
             F::destructibility >= constraint_level::nontrivial &&
             F::copyability != constraint_level::trivial)
  {
    if (this != std::addressof(rhs)) [[likely]] {
      reset();
      initialize(std::move(rhs));
    }
    return *this;
  }
  template <class P>
  constexpr proxy& operator=(P&& ptr) noexcept(
      std::is_nothrow_constructible_v<std::decay_t<P>, P> &&
      F::destructibility >= constraint_level::nothrow)
    requires(details::ptr_traits<std::decay_t<P>>::applicable &&
             std::is_constructible_v<std::decay_t<P>, P> &&
             F::destructibility >= constraint_level::nontrivial)
  {
    if constexpr (std::is_nothrow_constructible_v<std::decay_t<P>, P>) {
      destroy();
      initialize<std::decay_t<P>>(std::forward<P>(ptr));
    } else {
      *this = proxy{std::forward<P>(ptr)};
    }
    return *this;
  }
  ~proxy()
    requires(F::destructibility == constraint_level::trivial)
  = default;
  ~proxy() noexcept(F::destructibility == constraint_level::nothrow)
    requires(F::destructibility == constraint_level::nontrivial ||
             F::destructibility == constraint_level::nothrow)
  {
    destroy();
  }

  bool has_value() const noexcept { return meta_.has_value(); }
  explicit operator bool() const noexcept { return meta_.has_value(); }
  void reset() noexcept(F::destructibility >= constraint_level::nothrow)
    requires(F::destructibility >= constraint_level::nontrivial)
  {
    destroy();
    initialize();
  }
  void swap(proxy& rhs) noexcept(F::relocatability >=
                                     constraint_level::nothrow ||
                                 F::copyability == constraint_level::trivial)
    requires(F::relocatability >= constraint_level::nontrivial ||
             F::copyability == constraint_level::trivial)
  {
    if constexpr (F::relocatability == constraint_level::trivial ||
                  F::copyability == constraint_level::trivial) {
      std::swap(meta_, rhs.meta_);
#ifdef __INTEL_LLVM_COMPILER
      // Workaround: Intel oneAPI compiler (as of 2025.2.0) may over-optimize
      // the swap below, causing unit tests failure
      std::byte temp[F::max_size];
      std::ranges::uninitialized_copy(ptr_, temp);
      std::ranges::uninitialized_copy(rhs.ptr_, ptr_);
      std::ranges::uninitialized_copy(temp, rhs.ptr_);
#else
      std::swap(ptr_, rhs.ptr_);
#endif // __INTEL_LLVM_COMPILER
    } else {
      if (meta_.has_value()) {
        if (rhs.meta_.has_value()) {
          proxy temp = std::move(*this);
          initialize(std::move(rhs));
          rhs.initialize(std::move(temp));
        } else {
          rhs.initialize(std::move(*this));
        }
      } else if (rhs.meta_.has_value()) {
        initialize(std::move(rhs));
      }
    }
  }
  template <class P, class... Args>
  constexpr P& emplace(Args&&... args) noexcept(
      std::is_nothrow_constructible_v<P, Args...> &&
      F::destructibility >= constraint_level::nothrow)
    requires(details::ptr_traits<P>::applicable &&
             std::is_constructible_v<P, Args...> &&
             F::destructibility >= constraint_level::nontrivial)
  {
    reset();
    return initialize<P>(std::forward<Args>(args)...);
  }
  template <class P, class U, class... Args>
  constexpr P& emplace(std::initializer_list<U> il, Args&&... args) noexcept(
      std::is_nothrow_constructible_v<P, std::initializer_list<U>&, Args...> &&
      F::destructibility >= constraint_level::nothrow)
    requires(details::ptr_traits<P>::applicable &&
             std::is_constructible_v<P, std::initializer_list<U>&, Args...> &&
             F::destructibility >= constraint_level::nontrivial)
  {
    reset();
    return initialize<P>(il, std::forward<Args>(args)...);
  }

  friend void swap(proxy& lhs, proxy& rhs) noexcept(noexcept(lhs.swap(rhs))) {
    lhs.swap(rhs);
  }
  friend bool operator==(const proxy& lhs, std::nullptr_t) noexcept {
    return !lhs.has_value();
  }

private:
  void initialize() {
    PRO4D_DEBUG(std::ignore = &pro_symbol_guard;)
    meta_.reset();
  }
  void initialize(const proxy& rhs)
    requires(F::copyability != constraint_level::none)
  {
    PRO4D_DEBUG(std::ignore = &pro_symbol_guard;)
    if (rhs.meta_.has_value()) {
      if constexpr (F::copyability == constraint_level::trivial) {
        std::ranges::uninitialized_copy(rhs.ptr_, ptr_);
        meta_ = rhs.meta_;
      } else {
        proxy_invoke<details::copy_dispatch,
                     void(proxy&) const noexcept(
                         F::copyability == constraint_level::nothrow)>(rhs,
                                                                       *this);
      }
    } else {
      meta_.reset();
    }
  }
  void initialize(proxy&& rhs)
    requires(F::relocatability != constraint_level::none)
  {
    PRO4D_DEBUG(std::ignore = &pro_symbol_guard;)
    if (rhs.meta_.has_value()) {
      if constexpr (F::relocatability == constraint_level::trivial) {
        std::ranges::uninitialized_copy(rhs.ptr_, ptr_);
        meta_ = rhs.meta_;
        rhs.meta_.reset();
      } else {
        proxy_invoke<details::relocate_dispatch,
                     void(proxy&) && noexcept(F::relocatability ==
                                              constraint_level::nothrow)>(
            std::move(rhs), *this);
      }
    } else {
      meta_.reset();
    }
  }
  template <class P, class... Args>
  constexpr P& initialize(Args&&... args) {
    PRO4D_DEBUG(std::ignore = &pro_symbol_guard;)
    P& result = *std::construct_at(reinterpret_cast<P*>(ptr_),
                                   std::forward<Args>(args)...);
    if constexpr (proxiable<P, F>) {
      meta_ = details::meta_ptr<typename details::facade_traits<F>::meta>{
          std::in_place_type<P>};
    } else {
      details::facade_traits<F>::template diagnose_proxiable<P>();
    }
    return result;
  }
  void destroy()
    requires(F::destructibility != constraint_level::none)
  {
    if constexpr (F::destructibility != constraint_level::trivial) {
      if (meta_.has_value()) {
        proxy_invoke<details::destroy_dispatch,
                     void() noexcept(F::destructibility ==
                                     constraint_level::nothrow)>(*this);
      }
    }
  }
  PRO4D_DEBUG(static inline void pro_symbol_guard(proxy& self,
                                                  const proxy& cself) {
    self.operator->();
    *self;
    *std::move(self);
    cself.operator->();
    *cself;
    *std::move(cself);
  })

  details::meta_ptr<typename details::facade_traits<F>::meta> meta_;
  alignas(F::max_align) std::byte ptr_[F::max_size];
};

template <class D, class O, facade F, class... Args>
auto proxy_invoke(proxy_indirect_accessor<F>& p, Args&&... args) ->
    typename details::overload_traits<O>::return_type {
  return details::invoke_impl<F, false, D, O>(
      details::as_proxy<F, details::qualifier_type::lv>(p),
      std::forward<Args>(args)...);
}
template <class D, class O, facade F, class... Args>
auto proxy_invoke(const proxy_indirect_accessor<F>& p, Args&&... args) ->
    typename details::overload_traits<O>::return_type {
  return details::invoke_impl<F, false, D, O>(
      details::as_proxy<F, details::qualifier_type::const_lv>(p),
      std::forward<Args>(args)...);
}
template <class D, class O, facade F, class... Args>
auto proxy_invoke(proxy_indirect_accessor<F>&& p, Args&&... args) ->
    typename details::overload_traits<O>::return_type {
  return details::invoke_impl<F, false, D, O>(
      details::as_proxy<F, details::qualifier_type::rv>(std::move(p)),
      std::forward<Args>(args)...);
}
template <class D, class O, facade F, class... Args>
auto proxy_invoke(const proxy_indirect_accessor<F>&& p, Args&&... args) ->
    typename details::overload_traits<O>::return_type {
  return details::invoke_impl<F, false, D, O>(
      details::as_proxy<F, details::qualifier_type::const_rv>(std::move(p)),
      std::forward<Args>(args)...);
}
template <class D, class O, facade F, class... Args>
auto proxy_invoke(proxy<F>& p, Args&&... args) ->
    typename details::overload_traits<O>::return_type {
  return details::invoke_impl<F, true, D, O>(p, std::forward<Args>(args)...);
}
template <class D, class O, facade F, class... Args>
auto proxy_invoke(const proxy<F>& p, Args&&... args) ->
    typename details::overload_traits<O>::return_type {
  return details::invoke_impl<F, true, D, O>(p, std::forward<Args>(args)...);
}
template <class D, class O, facade F, class... Args>
auto proxy_invoke(proxy<F>&& p, Args&&... args) ->
    typename details::overload_traits<O>::return_type {
  return details::invoke_impl<F, true, D, O>(std::move(p),
                                             std::forward<Args>(args)...);
}
template <class D, class O, facade F, class... Args>
auto proxy_invoke(const proxy<F>&& p, Args&&... args) ->
    typename details::overload_traits<O>::return_type {
  return details::invoke_impl<F, true, D, O>(std::move(p),
                                             std::forward<Args>(args)...);
}

template <class R, facade F>
const R& proxy_reflect(const proxy_indirect_accessor<F>& p) noexcept {
  return static_cast<const details::refl_meta<false, R>&>(
             details::proxy_helper::get_meta(
                 details::as_proxy<F, details::qualifier_type::const_lv>(p)))
      .reflector;
}
template <class R, facade F>
const R& proxy_reflect(const proxy<F>& p) noexcept {
  return static_cast<const details::refl_meta<true, R>&>(
             details::proxy_helper::get_meta(p))
      .reflector;
}

// =============================================================================
// == Core Extensions (substitution_dispatch, proxy_view, weak_proxy)         ==
// =============================================================================

struct substitution_dispatch;

template <facade F>
struct observer_facade;
template <facade F>
using proxy_view = proxy<observer_facade<F>>;

template <facade F>
struct weak_facade;
template <facade F>
using weak_proxy = proxy<weak_facade<F>>;

namespace details {

template <class F>
struct converter {
  explicit converter(F f) noexcept : f_(std::move(f)) {}
  converter(const converter&) = delete;
  template <class T>
  operator T() && noexcept(
      std::is_nothrow_invocable_r_v<T, F, std::in_place_type_t<T>>)
    requires(std::is_invocable_r_v<T, F, std::in_place_type_t<T>>)
  {
    return std::move(f_)(std::in_place_type<T>);
  }

private:
  F f_;
};

#define PROD_DEF_CAST_ACCESSOR(oq, pq, ne, ...)                                \
  template <class P, class D, class T>                                         \
  struct accessor<P, D, T() oq ne> {                                           \
    PRO4D_GEN_DEBUG_SYMBOL_FOR_MEM_ACCESSOR(operator T)                        \
    explicit(Expl) operator T() oq ne {                                        \
      if constexpr (Nullable) {                                                \
        if (!static_cast<const P&>(*this).has_value()) {                       \
          return nullptr;                                                      \
        }                                                                      \
      }                                                                        \
      return proxy_invoke<D, T() oq ne>(static_cast<P pq>(*this));             \
    }                                                                          \
  }
template <bool Expl, bool Nullable>
struct cast_dispatch_base {
  PRO4D_DEF_ACCESSOR_TEMPLATE(
      MEM, PROD_DEF_CAST_ACCESSOR,
      operator typename overload_traits<ProOs>::return_type)
};
#undef PROD_DEF_CAST_ACCESSOR

template <bool IsDirect, class D, class... Os>
struct conv_impl {
  static constexpr bool is_direct = IsDirect;
  using dispatch_type = D;
  using overload_types = std::tuple<Os...>;
};
template <bool IsDirect, class R>
struct refl_impl {
  static constexpr bool is_direct = IsDirect;
  using reflector_type = R;
};
template <class Cs, class Rs, std::size_t MaxSize, std::size_t MaxAlign,
          constraint_level Copyability, constraint_level Relocatability,
          constraint_level Destructibility>
struct facade_impl {
  using convention_types = Cs;
  using reflection_types = Rs;
  static constexpr std::size_t max_size = MaxSize;
  static constexpr std::size_t max_align = MaxAlign;
  static constexpr constraint_level copyability = Copyability;
  static constexpr constraint_level relocatability = Relocatability;
  static constexpr constraint_level destructibility = Destructibility;
};

template <class O, class I>
struct add_tuple_reduction : std::type_identity<O> {};
template <class... Os, class I>
  requires(!std::is_same_v<I, Os> && ...)
struct add_tuple_reduction<std::tuple<Os...>, I>
    : std::type_identity<std::tuple<Os..., I>> {};
template <class O, class... Is>
using add_tuple_t =
    recursive_reduction_t<reduction_traits<add_tuple_reduction>::template type,
                          O, Is...>;

template <bool IsDirect, class D>
struct conv_instantiation_helper {
  template <class... Os>
  using type = conv_impl<IsDirect, D, Os...>;
};
template <bool IsDirect, class D, class Os>
using instantiated_conv_t =
    instantiated_t<conv_instantiation_helper<IsDirect, D>::template type, Os>;

template <class O>
using observer_substitution_overload =
    proxy_view<typename overload_traits<O>::return_type::facade_type>()
        const noexcept;
template <class... Os>
using observer_substitution_conv = instantiated_conv_t<
    true, substitution_dispatch,
    add_tuple_t<std::tuple<>, observer_substitution_overload<Os>...>>;

template <class C>
struct observer_conv_traits : std::type_identity<void> {};
template <class C>
  requires(C::is_direct &&
           std::is_same_v<typename C::dispatch_type, substitution_dispatch>)
struct observer_conv_traits<C>
    : std::type_identity<instantiated_t<observer_substitution_conv,
                                        typename C::overload_types>> {};
template <class C>
  requires(!C::is_direct)
struct observer_conv_traits<C> : std::type_identity<C> {};
template <class... Cs>
using observer_conv_types =
    composite_t<std::tuple<>, typename observer_conv_traits<Cs>::type...>;
template <class... Rs>
using observer_refl_types =
    composite_t<std::tuple<>, std::conditional_t<Rs::is_direct, void, Rs>...>;

template <class P>
auto weak_lock_impl(const P& self) noexcept
  requires(requires { self.lock(); })
{
  if constexpr (std::is_constructible_v<bool, decltype(self.lock())>) {
    return converter{
        [&self]<class F>(std::in_place_type_t<proxy<F>>) noexcept -> proxy<F> {
          auto strong = self.lock();
          return strong ? proxy<F>{std::move(strong)} : proxy<F>{};
        }};
  } else {
    return self.lock();
  }
}
PRO4_DEF_FREE_AS_MEM_DISPATCH(weak_mem_lock, weak_lock_impl, lock);

template <class O>
struct weak_substitution_overload_traits;
#define PROD_DEF_WEAK_SUBSTITUTION_OVERLOAD_TRAITS(oq, pq, ne, ...)            \
  template <class F>                                                           \
  struct weak_substitution_overload_traits<proxy<F>() oq ne>                   \
      : std::type_identity<weak_proxy<F>() oq ne> {};
PRO4D_DEF_OVERLOAD_SPECIALIZATIONS(PROD_DEF_WEAK_SUBSTITUTION_OVERLOAD_TRAITS)
#undef PROD_DEF_WEAK_SUBSTITUTION_OVERLOAD_TRAITS
template <class... Os>
using weak_substitution_conv =
    conv_impl<true, substitution_dispatch,
              typename weak_substitution_overload_traits<Os>::type...>;

template <class C>
struct weak_conv_traits : std::type_identity<void> {};
template <class C>
  requires(C::is_direct &&
           std::is_same_v<typename C::dispatch_type, substitution_dispatch>)
struct weak_conv_traits<C>
    : std::type_identity<
          instantiated_t<weak_substitution_conv, typename C::overload_types>> {
};
template <class F, class... Cs>
using weak_conv_types = composite_t<
    std::tuple<conv_impl<true, weak_mem_lock, proxy<F>() const noexcept>>,
    typename weak_conv_traits<Cs>::type...>;

} // namespace details

struct PRO4D_ENFORCE_EBO substitution_dispatch
    : details::cast_dispatch_base<false, true>,
      details::internal_dispatch {
  template <class P, class F1>
  PRO4D_STATIC_CALL(auto, std::in_place_type_t<P>, proxy<F1>&& self) noexcept {
    return details::converter{
        [&self]<class F2>(std::in_place_type_t<proxy<F2>>) noexcept {
          proxy<F2> ret;
          details::proxy_helper::trivially_relocate<P>(self, ret);
          return ret;
        }};
  }
  template <class T>
  PRO4D_STATIC_CALL(T&&, T&& self) noexcept {
    return std::forward<T>(self);
  }

  // This overload is not reachable at runtime, but is necessary to ensure
  // substitution_dispatch is SFINAE-friendly.
  template <class T>
  PRO4D_STATIC_CALL(auto, T&&) noexcept
    requires(std::is_same_v<T, std::remove_cvref_t<T>> &&
             is_bitwise_trivially_relocatable_v<T>)
  {
    return details::converter{
        []<class F2>(std::in_place_type_t<proxy<F2>>) noexcept -> proxy<F2>
          requires(proxiable<T, F2>)
        { PROD_UNREACHABLE(); }};
  }
};

template <facade F>
struct observer_facade
    : details::facade_impl<
          details::instantiated_t<details::observer_conv_types,
                                  typename F::convention_types>,
          details::instantiated_t<details::observer_refl_types,
                                  typename F::reflection_types>,
          sizeof(void*), alignof(void*), constraint_level::trivial,
          constraint_level::trivial, constraint_level::trivial> {};

template <facade F>
struct weak_facade
    : details::facade_impl<
          details::instantiated_t<details::weak_conv_types,
                                  typename F::convention_types, F>,
          std::tuple<>, F::max_size, F::max_align, F::copyability,
          F::relocatability, F::destructibility> {};

// =============================================================================
// == Proxy Creation (make_proxy, proxiable_target, etc.)                     ==
// =============================================================================

namespace details {

template <class LR, class CLR, class RR, class CRR>
class observer_ptr {
public:
  explicit observer_ptr(LR lr) : lr_(lr) {}
  observer_ptr(const observer_ptr&) = default;
  auto operator->() noexcept { return std::addressof(lr_); }
  auto operator->() const noexcept {
    return std::addressof(static_cast<CLR>(lr_));
  }
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
  auto al =
      typename std::allocator_traits<Alloc>::template rebind_alloc<T>(alloc);
  auto deleter = [&](T* ptr) { al.deallocate(ptr, 1); };
  std::unique_ptr<T, decltype(deleter)> result{al.allocate(1), deleter};
  std::construct_at(result.get(), std::forward<Args>(args)...);
  return result.release();
}
template <class Alloc, class T>
void deallocate(const Alloc& alloc, T* ptr) {
  auto al =
      typename std::allocator_traits<Alloc>::template rebind_alloc<T>(alloc);
  std::destroy_at(ptr);
  al.deallocate(ptr, 1);
}
template <class Alloc>
struct alloc_aware {
  explicit alloc_aware(const Alloc& alloc) noexcept : alloc(alloc) {}
  alloc_aware(const alloc_aware&) noexcept = default;

  [[PROD_NO_UNIQUE_ADDRESS_ATTRIBUTE]]
  Alloc alloc;
};
template <class T>
class indirect_ptr {
public:
  explicit indirect_ptr(T* ptr) noexcept : ptr_(ptr) {}
  auto operator->() noexcept { return std::addressof(**ptr_); }
  auto operator->() const noexcept { return std::addressof(**ptr_); }
  decltype(auto) operator*() & noexcept { return **ptr_; }
  decltype(auto) operator*() const& noexcept { return *std::as_const(*ptr_); }
  decltype(auto) operator*() && noexcept { return *std::move(*ptr_); }
  decltype(auto) operator*() const&& noexcept {
    return *std::move(std::as_const(*ptr_));
  }

protected:
  T* ptr_;
};

template <class T, class Alloc>
class PRO4D_ENFORCE_EBO allocated_ptr : private alloc_aware<Alloc>,
                                        public indirect_ptr<inplace_ptr<T>> {
public:
  template <class... Args>
  allocated_ptr(const Alloc& alloc, Args&&... args)
      : alloc_aware<Alloc>(alloc),
        indirect_ptr<inplace_ptr<T>>(allocate<inplace_ptr<T>>(
            this->alloc, std::in_place, std::forward<Args>(args)...)) {}
  allocated_ptr(const allocated_ptr& rhs)
    requires(std::is_copy_constructible_v<T>)
      : alloc_aware<Alloc>(rhs),
        indirect_ptr<inplace_ptr<T>>(
            allocate<inplace_ptr<T>>(this->alloc, std::in_place, *rhs)) {}
  allocated_ptr(allocated_ptr&& rhs) = delete;
  ~allocated_ptr() noexcept(std::is_nothrow_destructible_v<T>) {
    deallocate(this->alloc, this->ptr_);
  }
};

template <class T, class Alloc>
struct PRO4D_ENFORCE_EBO compact_ptr_storage : alloc_aware<Alloc>,
                                               inplace_ptr<T> {
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
      : indirect_ptr<Storage>(
            allocate<Storage>(alloc, alloc, std::forward<Args>(args)...)) {}
  compact_ptr(const compact_ptr& rhs)
    requires(std::is_copy_constructible_v<T>)
      : indirect_ptr<Storage>(
            allocate<Storage>(rhs.ptr_->alloc, rhs.ptr_->alloc, *rhs)) {}
  compact_ptr(compact_ptr&& rhs) = delete;
  ~compact_ptr() noexcept(std::is_nothrow_destructible_v<T>) {
    deallocate(this->ptr_->alloc, this->ptr_);
  }
};

struct shared_compact_ptr_storage_base {
  std::atomic_long ref_count = 1;
};
template <class T, class Alloc>
struct PRO4D_ENFORCE_EBO shared_compact_ptr_storage
    : shared_compact_ptr_storage_base,
      alloc_aware<Alloc>,
      inplace_ptr<T> {
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
      : indirect_ptr<Storage>(
            allocate<Storage>(alloc, alloc, std::forward<Args>(args)...)) {}
  shared_compact_ptr(const shared_compact_ptr& rhs) noexcept
      : indirect_ptr<Storage>(rhs.ptr_) {
    this->ptr_->ref_count.fetch_add(1, std::memory_order::relaxed);
  }
  shared_compact_ptr(shared_compact_ptr&& rhs) = delete;
  ~shared_compact_ptr() noexcept(std::is_nothrow_destructible_v<T>) {
    if (this->ptr_->ref_count.fetch_sub(1, std::memory_order::acq_rel) == 1) {
      deallocate(this->ptr_->alloc, this->ptr_);
    }
  }
};

struct strong_weak_compact_ptr_storage_base {
  std::atomic_long strong_count = 1, weak_count = 1;
};
template <class T, class Alloc>
struct strong_weak_compact_ptr_storage : strong_weak_compact_ptr_storage_base,
                                         alloc_aware<Alloc> {
  template <class... Args>
  explicit strong_weak_compact_ptr_storage(const Alloc& alloc, Args&&... args)
      : alloc_aware<Alloc>(alloc) {
    std::construct_at(reinterpret_cast<T*>(&value),
                      std::forward<Args>(args)...);
  }

  alignas(alignof(T)) std::byte value[sizeof(T)];
};
template <class T, class Alloc>
class weak_compact_ptr;
template <class T, class Alloc>
class strong_compact_ptr {
  using Storage = strong_weak_compact_ptr_storage<T, Alloc>;
  friend class weak_compact_ptr<T, Alloc>;

public:
  using weak_type = weak_compact_ptr<T, Alloc>;

  explicit strong_compact_ptr(Storage* ptr) noexcept : ptr_(ptr) {}
  template <class... Args>
  strong_compact_ptr(const Alloc& alloc, Args&&... args)
      : ptr_(allocate<Storage>(alloc, alloc, std::forward<Args>(args)...)) {}
  strong_compact_ptr(const strong_compact_ptr& rhs) noexcept : ptr_(rhs.ptr_) {
    ptr_->strong_count.fetch_add(1, std::memory_order::relaxed);
  }
  strong_compact_ptr(strong_compact_ptr&& rhs) = delete;
  ~strong_compact_ptr() noexcept(std::is_nothrow_destructible_v<T>) {
    if (ptr_->strong_count.fetch_sub(1, std::memory_order::acq_rel) == 1) {
      std::destroy_at(operator->());
      if (ptr_->weak_count.fetch_sub(1u, std::memory_order::release) == 1) {
        deallocate(ptr_->alloc, ptr_);
      }
    }
  }
  T* operator->() noexcept {
    return std::launder(reinterpret_cast<T*>(&ptr_->value));
  }
  const T* operator->() const noexcept {
    return std::launder(reinterpret_cast<const T*>(&ptr_->value));
  }
  T& operator*() & noexcept { return *operator->(); }
  const T& operator*() const& noexcept { return *operator->(); }
  T&& operator*() && noexcept { return std::move(*operator->()); }
  const T&& operator*() const&& noexcept { return std::move(*operator->()); }

private:
  strong_weak_compact_ptr_storage<T, Alloc>* ptr_;
};
template <class T, class Alloc>
class weak_compact_ptr {
public:
  using element_type = T;

  weak_compact_ptr(const strong_compact_ptr<T, Alloc>& rhs) noexcept
      : ptr_(rhs.ptr_) {
    ptr_->weak_count.fetch_add(1, std::memory_order::relaxed);
  }
  weak_compact_ptr(const weak_compact_ptr& rhs) noexcept : ptr_(rhs.ptr_) {
    ptr_->weak_count.fetch_add(1, std::memory_order::relaxed);
  }
  weak_compact_ptr(weak_compact_ptr&& rhs) = delete;
  ~weak_compact_ptr() noexcept {
    if (ptr_->weak_count.fetch_sub(1u, std::memory_order::acq_rel) == 1) {
      deallocate(ptr_->alloc, ptr_);
    }
  }
  auto lock() const noexcept {
    return converter{[ptr = this->ptr_]<class F>(
                         std::in_place_type_t<proxy<F>>) noexcept -> proxy<F> {
      long ref_count = ptr->strong_count.load(std::memory_order::relaxed);
      do {
        if (ref_count == 0) {
          return proxy<F>{};
        }
      } while (!ptr->strong_count.compare_exchange_weak(
          ref_count, ref_count + 1, std::memory_order::relaxed));
      return proxy<F>{std::in_place_type<strong_compact_ptr<T, Alloc>>, ptr};
    }};
  }

private:
  strong_weak_compact_ptr_storage<T, Alloc>* ptr_;
};

template <class F, class T, class Alloc, class... Args>
constexpr proxy<F> allocate_proxy_impl(const Alloc& alloc, Args&&... args) {
  if constexpr (proxiable<allocated_ptr<T, Alloc>, F>) {
    return proxy<F>{std::in_place_type<allocated_ptr<T, Alloc>>, alloc,
                    std::forward<Args>(args)...};
  } else {
    return proxy<F>{std::in_place_type<compact_ptr<T, Alloc>>, alloc,
                    std::forward<Args>(args)...};
  }
}
template <class F, class T, class... Args>
constexpr proxy<F> make_proxy_impl(Args&&... args) {
  if constexpr (proxiable<inplace_ptr<T>, F>) {
    return proxy<F>{std::in_place_type<inplace_ptr<T>>, std::in_place,
                    std::forward<Args>(args)...};
  } else {
    return allocate_proxy_impl<F, T>(std::allocator<void>{},
                                     std::forward<Args>(args)...);
  }
}
template <class F, class T, class Alloc, class... Args>
constexpr proxy<F> allocate_proxy_shared_impl(const Alloc& alloc,
                                              Args&&... args) {
  if constexpr (std::is_convertible_v<proxy<F>, weak_proxy<F>>) {
    return proxy<F>{std::in_place_type<strong_compact_ptr<T, Alloc>>, alloc,
                    std::forward<Args>(args)...};
  } else {
    return proxy<F>{std::in_place_type<shared_compact_ptr<T, Alloc>>, alloc,
                    std::forward<Args>(args)...};
  }
}
template <class F, class T, class... Args>
constexpr proxy<F> make_proxy_shared_impl(Args&&... args) {
  return allocate_proxy_shared_impl<F, T>(std::allocator<void>{},
                                          std::forward<Args>(args)...);
}
#endif // __STDC_HOSTED__

} // namespace details

template <class T, class F>
concept inplace_proxiable_target = proxiable<details::inplace_ptr<T>, F>;

template <class T, class F>
concept proxiable_target =
    proxiable<details::observer_ptr<T&, const T&, T&&, const T&&>,
              observer_facade<F>>;

template <class T>
  requires(is_bitwise_trivially_relocatable_v<T>)
struct is_bitwise_trivially_relocatable<details::inplace_ptr<T>>
    : std::true_type {};

template <facade F, class T, class... Args>
constexpr proxy<F> make_proxy_inplace(Args&&... args) noexcept(
    std::is_nothrow_constructible_v<T, Args...>)
  requires(std::is_constructible_v<T, Args...>)
{
  return proxy<F>{std::in_place_type<details::inplace_ptr<T>>, std::in_place,
                  std::forward<Args>(args)...};
}
template <facade F, class T, class U, class... Args>
constexpr proxy<F>
    make_proxy_inplace(std::initializer_list<U> il, Args&&... args) noexcept(
        std::is_nothrow_constructible_v<T, std::initializer_list<U>&, Args...>)
  requires(std::is_constructible_v<T, std::initializer_list<U>&, Args...>)
{
  return proxy<F>{std::in_place_type<details::inplace_ptr<T>>, std::in_place,
                  il, std::forward<Args>(args)...};
}
template <facade F, class T>
constexpr proxy<F> make_proxy_inplace(T&& value) noexcept(
    std::is_nothrow_constructible_v<std::decay_t<T>, T>)
  requires(std::is_constructible_v<std::decay_t<T>, T>)
{
  return proxy<F>{std::in_place_type<details::inplace_ptr<std::decay_t<T>>>,
                  std::in_place, std::forward<T>(value)};
}

template <facade F, class T>
constexpr proxy_view<F> make_proxy_view(T& value) noexcept {
  return proxy_view<F>{
      details::observer_ptr<T&, const T&, T&&, const T&&>{value}};
}

#if __STDC_HOSTED__
template <class T, class D>
  requires(is_bitwise_trivially_relocatable_v<D>)
struct is_bitwise_trivially_relocatable<std::unique_ptr<T, D>>
    : std::true_type {};
template <class T>
struct is_bitwise_trivially_relocatable<std::shared_ptr<T>> : std::true_type {};
template <class T>
struct is_bitwise_trivially_relocatable<std::weak_ptr<T>> : std::true_type {};
template <class T, class Alloc>
  requires(is_bitwise_trivially_relocatable_v<Alloc>)
struct is_bitwise_trivially_relocatable<details::allocated_ptr<T, Alloc>>
    : std::true_type {};
template <class T, class Alloc>
struct is_bitwise_trivially_relocatable<details::compact_ptr<T, Alloc>>
    : std::true_type {};
template <class T, class Alloc>
struct is_bitwise_trivially_relocatable<details::shared_compact_ptr<T, Alloc>>
    : std::true_type {};
template <class T, class Alloc>
struct is_bitwise_trivially_relocatable<details::strong_compact_ptr<T, Alloc>>
    : std::true_type {};
template <class T, class Alloc>
struct is_bitwise_trivially_relocatable<details::weak_compact_ptr<T, Alloc>>
    : std::true_type {};

template <facade F, class T, class Alloc, class... Args>
constexpr proxy<F> allocate_proxy(const Alloc& alloc, Args&&... args)
  requires(std::is_constructible_v<T, Args...>)
{
  return details::allocate_proxy_impl<F, T>(alloc, std::forward<Args>(args)...);
}
template <facade F, class T, class Alloc, class U, class... Args>
constexpr proxy<F> allocate_proxy(const Alloc& alloc,
                                  std::initializer_list<U> il, Args&&... args)
  requires(std::is_constructible_v<T, std::initializer_list<U>&, Args...>)
{
  return details::allocate_proxy_impl<F, T>(alloc, il,
                                            std::forward<Args>(args)...);
}
template <facade F, class Alloc, class T>
constexpr proxy<F> allocate_proxy(const Alloc& alloc, T&& value)
  requires(std::is_constructible_v<std::decay_t<T>, T>)
{
  return details::allocate_proxy_impl<F, std::decay_t<T>>(
      alloc, std::forward<T>(value));
}
template <facade F, class T, class... Args>
constexpr proxy<F> make_proxy(Args&&... args)
  requires(std::is_constructible_v<T, Args...>)
{
  return details::make_proxy_impl<F, T>(std::forward<Args>(args)...);
}
template <facade F, class T, class U, class... Args>
constexpr proxy<F> make_proxy(std::initializer_list<U> il, Args&&... args)
  requires(std::is_constructible_v<T, std::initializer_list<U>&, Args...>)
{
  return details::make_proxy_impl<F, T>(il, std::forward<Args>(args)...);
}
template <facade F, class T>
constexpr proxy<F> make_proxy(T&& value)
  requires(std::is_constructible_v<std::decay_t<T>, T>)
{
  return details::make_proxy_impl<F, std::decay_t<T>>(std::forward<T>(value));
}

template <facade F, class T, class Alloc, class... Args>
constexpr proxy<F> allocate_proxy_shared(const Alloc& alloc, Args&&... args)
  requires(std::is_constructible_v<T, Args...>)
{
  return details::allocate_proxy_shared_impl<F, T>(alloc,
                                                   std::forward<Args>(args)...);
}
template <facade F, class T, class Alloc, class U, class... Args>
constexpr proxy<F> allocate_proxy_shared(const Alloc& alloc,
                                         std::initializer_list<U> il,
                                         Args&&... args)
  requires(std::is_constructible_v<T, std::initializer_list<U>&, Args...>)
{
  return details::allocate_proxy_shared_impl<F, T>(alloc, il,
                                                   std::forward<Args>(args)...);
}
template <facade F, class Alloc, class T>
constexpr proxy<F> allocate_proxy_shared(const Alloc& alloc, T&& value)
  requires(std::is_constructible_v<std::decay_t<T>, T>)
{
  return details::allocate_proxy_shared_impl<F, std::decay_t<T>>(
      alloc, std::forward<T>(value));
}
template <facade F, class T, class... Args>
constexpr proxy<F> make_proxy_shared(Args&&... args)
  requires(std::is_constructible_v<T, Args...>)
{
  return details::make_proxy_shared_impl<F, T>(std::forward<Args>(args)...);
}
template <facade F, class T, class U, class... Args>
constexpr proxy<F> make_proxy_shared(std::initializer_list<U> il,
                                     Args&&... args)
  requires(std::is_constructible_v<T, std::initializer_list<U>&, Args...>)
{
  return details::make_proxy_shared_impl<F, T>(il, std::forward<Args>(args)...);
}
template <facade F, class T>
constexpr proxy<F> make_proxy_shared(T&& value)
  requires(std::is_constructible_v<std::decay_t<T>, T>)
{
  return details::make_proxy_shared_impl<F, std::decay_t<T>>(
      std::forward<T>(value));
}
#endif // __STDC_HOSTED__

// =============================================================================
// == Facade Creation (facade_builder)                                        ==
// =============================================================================

namespace details {

inline constexpr std::size_t invalid_size =
    std::numeric_limits<std::size_t>::max();
inline constexpr constraint_level invalid_cl = static_cast<constraint_level>(
    std::numeric_limits<std::underlying_type_t<constraint_level>>::min());
consteval std::size_t merge_size(std::size_t a, std::size_t b) {
  return a < b ? a : b;
}
consteval constraint_level merge_constraint(constraint_level a,
                                            constraint_level b) {
  return a < b ? b : a;
}
consteval std::size_t max_align_of(std::size_t value) {
  value &= ~value + 1u;
  return value < alignof(std::max_align_t) ? value : alignof(std::max_align_t);
}

template <class T, class U>
using merge_tuple_t = instantiated_t<add_tuple_t, U, T>;
template <class C1, class C2>
using merge_conv_t = instantiated_conv_t<
    C1::is_direct, typename C1::dispatch_type,
    merge_tuple_t<typename C1::overload_types, typename C2::overload_types>>;

template <class Cs1, class C2, class C>
struct add_conv_reduction;
template <class... Cs1, class C2, class... Cs3, class C>
struct add_conv_reduction<std::tuple<Cs1...>, std::tuple<C2, Cs3...>, C>
    : add_conv_reduction<std::tuple<Cs1..., C2>, std::tuple<Cs3...>, C> {};
template <class... Cs1, class C2, class... Cs3, class C>
  requires(
      C::is_direct == C2::is_direct &&
      std::is_same_v<typename C::dispatch_type, typename C2::dispatch_type>)
struct add_conv_reduction<std::tuple<Cs1...>, std::tuple<C2, Cs3...>, C>
    : std::type_identity<std::tuple<Cs1..., merge_conv_t<C2, C>, Cs3...>> {};
template <class... Cs, class C>
struct add_conv_reduction<std::tuple<Cs...>, std::tuple<>, C>
    : std::type_identity<std::tuple<
          Cs..., merge_conv_t<
                     conv_impl<C::is_direct, typename C::dispatch_type>, C>>> {
};
template <class Cs, class C>
using add_conv_t = typename add_conv_reduction<std::tuple<>, Cs, C>::type;

template <class F, constraint_level CL>
using copy_conversion_overload =
    proxy<F>() const& noexcept(CL >= constraint_level::nothrow);
template <class F, constraint_level CL>
using move_conversion_overload =
    proxy<F>() && noexcept(CL >= constraint_level::nothrow);
template <class Cs, class F, constraint_level CCL, constraint_level RCL>
struct add_substitution_conv
    : std::type_identity<add_conv_t<
          Cs, instantiated_conv_t<
                  true, substitution_dispatch,
                  composite_t<
                      std::tuple<>,
                      std::conditional_t<CCL == constraint_level::none, void,
                                         copy_conversion_overload<F, CCL>>,
                      std::conditional_t<RCL == constraint_level::none, void,
                                         move_conversion_overload<F, RCL>>>>>> {
};
template <class Cs, class F>
struct add_substitution_conv<Cs, F, constraint_level::none,
                             constraint_level::none> : std::type_identity<Cs> {
};

template <class Cs1, class... Cs2>
using merge_conv_tuple_t = recursive_reduction_t<add_conv_t, Cs1, Cs2...>;
template <class Cs, class F, bool WithSubstitution>
using merge_facade_conv_t = typename add_substitution_conv<
    instantiated_t<merge_conv_tuple_t, typename F::convention_types, Cs>, F,
    WithSubstitution ? F::copyability : constraint_level::none,
    (WithSubstitution && F::copyability != constraint_level::trivial)
        ? F::relocatability
        : constraint_level::none>::type;

} // namespace details

template <class Cs, class Rs, std::size_t MaxSize, std::size_t MaxAlign,
          constraint_level Copyability, constraint_level Relocatability,
          constraint_level Destructibility>
struct basic_facade_builder {
  template <class D, details::extended_overload... Os>
    requires(sizeof...(Os) > 0u)
  using add_indirect_convention = basic_facade_builder<
      details::add_conv_t<Cs, details::conv_impl<false, D, Os...>>, Rs, MaxSize,
      MaxAlign, Copyability, Relocatability, Destructibility>;
  template <class D, details::extended_overload... Os>
    requires(sizeof...(Os) > 0u)
  using add_direct_convention = basic_facade_builder<
      details::add_conv_t<Cs, details::conv_impl<true, D, Os...>>, Rs, MaxSize,
      MaxAlign, Copyability, Relocatability, Destructibility>;
  template <class D, details::extended_overload... Os>
    requires(sizeof...(Os) > 0u)
  using add_convention = add_indirect_convention<D, Os...>;
  template <class R>
  using add_indirect_reflection = basic_facade_builder<
      Cs, details::add_tuple_t<Rs, details::refl_impl<false, R>>, MaxSize,
      MaxAlign, Copyability, Relocatability, Destructibility>;
  template <class R>
  using add_direct_reflection = basic_facade_builder<
      Cs, details::add_tuple_t<Rs, details::refl_impl<true, R>>, MaxSize,
      MaxAlign, Copyability, Relocatability, Destructibility>;
  template <class R>
  using add_reflection = add_indirect_reflection<R>;
  template <facade F, bool WithSubstitution = false>
  using add_facade = basic_facade_builder<
      details::merge_facade_conv_t<Cs, F, WithSubstitution>,
      details::merge_tuple_t<Rs, typename F::reflection_types>,
      details::merge_size(MaxSize, F::max_size),
      details::merge_size(MaxAlign, F::max_align),
      details::merge_constraint(Copyability, F::copyability),
      details::merge_constraint(Relocatability, F::relocatability),
      details::merge_constraint(Destructibility, F::destructibility)>;
  template <std::size_t PtrSize,
            std::size_t PtrAlign = details::max_align_of(PtrSize)>
    requires(details::is_layout_well_formed(PtrSize, PtrAlign))
  using restrict_layout =
      basic_facade_builder<Cs, Rs, details::merge_size(MaxSize, PtrSize),
                           details::merge_size(MaxAlign, PtrAlign), Copyability,
                           Relocatability, Destructibility>;
  template <constraint_level CL>
    requires(details::is_cl_well_formed(CL))
  using support_copy =
      basic_facade_builder<Cs, Rs, MaxSize, MaxAlign,
                           details::merge_constraint(Copyability, CL),
                           Relocatability, Destructibility>;
  template <constraint_level CL>
    requires(details::is_cl_well_formed(CL))
  using support_relocation =
      basic_facade_builder<Cs, Rs, MaxSize, MaxAlign, Copyability,
                           details::merge_constraint(Relocatability, CL),
                           Destructibility>;
  template <constraint_level CL>
    requires(details::is_cl_well_formed(CL))
  using support_destruction =
      basic_facade_builder<Cs, Rs, MaxSize, MaxAlign, Copyability,
                           Relocatability,
                           details::merge_constraint(Destructibility, CL)>;
  template <template <class> class Skill>
  using add_skill = Skill<basic_facade_builder>;
  using build = details::facade_impl<
      Cs, Rs,
      MaxSize == details::invalid_size ? sizeof(details::ptr_prototype)
                                       : MaxSize,
      MaxAlign == details::invalid_size ? alignof(details::ptr_prototype)
                                        : MaxAlign,
      Copyability == details::invalid_cl ? constraint_level::none : Copyability,
      Relocatability == details::invalid_cl ? constraint_level::trivial
                                            : Relocatability,
      Destructibility == details::invalid_cl ? constraint_level::nothrow
                                             : Destructibility>;
  basic_facade_builder() = delete;
};
using facade_builder =
    basic_facade_builder<std::tuple<>, std::tuple<>, details::invalid_size,
                         details::invalid_size, details::invalid_cl,
                         details::invalid_cl, details::invalid_cl>;

// =============================================================================
// == Skill Extensions (skills::rtti, skills::format, etc.)                   ==
// =============================================================================

#if __cpp_rtti >= 199711L
class bad_proxy_cast : public std::bad_cast {
public:
  char const* what() const noexcept override {
    return "pro::v4::bad_proxy_cast";
  }
};
#endif // __cpp_rtti >= 199711L

namespace details {

struct view_conversion_dispatch : cast_dispatch_base<false, true> {
  template <class T>
  PRO4D_STATIC_CALL(auto, T& value) noexcept
    requires(requires {
      { std::addressof(*value) } noexcept;
    })
  {
    return observer_ptr<decltype(*value), decltype(*std::as_const(value)),
                        decltype(*std::move(value)),
                        decltype(*std::move(std::as_const(value)))>{*value};
  }
};
template <class F>
using view_conversion_overload = proxy_view<F>() & noexcept;

struct weak_conversion_dispatch : cast_dispatch_base<false, true> {
  template <class P>
  PRO4D_STATIC_CALL(auto, const P& self) noexcept
    requires(requires(const typename P::weak_type& w) {
      { w.lock() } noexcept;
    } && std::is_convertible_v<const P&, typename P::weak_type>)
  {
    return converter{
        [&self]<class F>(std::in_place_type_t<proxy<F>>) noexcept
          requires(proxiable<typename P::weak_type, F>)
        { return proxy<F>{std::in_place_type<typename P::weak_type>, self}; }};
  }
};
template <class F>
using weak_conversion_overload = weak_proxy<F>() const noexcept;

#ifdef PRO4D_HAS_FORMAT
template <class CharT>
struct format_overload_traits;
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
  PRO4D_STATIC_CALL(OutIt, const T& self, std::basic_string_view<CharT> spec,
                    std::basic_format_context<OutIt, CharT>& fc)
    requires(
#if __cpp_lib_format_ranges >= 202207L
        std::formattable<T, CharT>
#else
        std::is_default_constructible_v<std::formatter<T, CharT>>
#endif // __cpp_lib_format_ranges >= 202207L
    )
  {
    std::formatter<T, CharT> impl;
    {
      std::basic_format_parse_context<CharT> pc{spec};
      impl.parse(pc);
    }
    return impl.format(self, fc);
  }
};
#endif // PRO4D_HAS_FORMAT

#if __cpp_rtti >= 199711L
struct proxy_cast_context {
  const std::type_info* type_ptr;
  bool is_ref;
  bool is_const;
  void* result_ptr;
};

template <class Self, class D, class O>
struct proxy_cast_accessor_impl {
  template <class T>
  friend T proxy_cast(Self self) {
    static_assert(!std::is_rvalue_reference_v<T>);
    if constexpr (std::is_lvalue_reference_v<T>) {
      using U = std::remove_reference_t<T>;
      void* result = nullptr;
      proxy_cast_context ctx{.type_ptr = &typeid(T),
                             .is_ref = true,
                             .is_const = std::is_const_v<U>,
                             .result_ptr = &result};
      proxy_invoke<D, O>(static_cast<Self>(self), ctx);
      if (result == nullptr) [[unlikely]] {
        PRO4D_THROW(bad_proxy_cast{});
      }
      return *static_cast<U*>(result);
    } else {
      std::optional<std::remove_const_t<T>> result;
      proxy_cast_context ctx{.type_ptr = &typeid(T),
                             .is_ref = false,
                             .is_const = false,
                             .result_ptr = &result};
      proxy_invoke<D, O>(static_cast<Self>(self), ctx);
      if (!result.has_value()) [[unlikely]] {
        PRO4D_THROW(bad_proxy_cast{});
      }
      return std::move(*result);
    }
  }
  template <class T>
  friend T* proxy_cast(std::remove_reference_t<Self>* self) noexcept
    requires(std::is_lvalue_reference<Self>::value)
  {
    void* result = nullptr;
    proxy_cast_context ctx{.type_ptr = &typeid(T),
                           .is_ref = true,
                           .is_const = std::is_const_v<T>,
                           .result_ptr = &result};
    proxy_invoke<D, O>(*self, ctx);
    return static_cast<T*>(result);
  }
};

#define PROD_DEF_PROXY_CAST_ACCESSOR(oq, pq, ne, ...)                          \
  template <class P, class D>                                                  \
  struct accessor<P, D, void(proxy_cast_context) oq ne>                        \
      : proxy_cast_accessor_impl<P pq, D, void(proxy_cast_context) oq ne> {}
struct proxy_cast_dispatch {
  template <class T>
  PRO4D_STATIC_CALL(void, T&& self, proxy_cast_context ctx) {
    if (typeid(T) == *ctx.type_ptr) [[likely]] {
      if (ctx.is_ref) {
        if constexpr (std::is_lvalue_reference_v<T>) {
          if (ctx.is_const || !std::is_const_v<T>) [[likely]] {
            *static_cast<void**>(ctx.result_ptr) = (void*)std::addressof(self);
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
  PRO4D_DEF_ACCESSOR_TEMPLATE(FREE, PROD_DEF_PROXY_CAST_ACCESSOR)
};
#undef PROD_DEF_PROXY_CAST_ACCESSOR

struct proxy_typeid_reflector {
  proxy_typeid_reflector() = default;
  template <class T>
  constexpr explicit proxy_typeid_reflector(std::in_place_type_t<T>)
      : info(&typeid(T)) {}

  template <class Self, class R>
  struct accessor {
    friend const std::type_info& proxy_typeid(const Self& self) noexcept {
      const proxy_typeid_reflector& refl = proxy_reflect<R>(self);
      return *refl.info;
    }
    PRO4D_DEBUG(
        accessor() noexcept { std::ignore = &pro_symbol_guard; }

        private : static inline const std::type_info& pro_symbol_guard(
            const Self& self) { return proxy_typeid(self); })
  };

  const std::type_info* info;
};
#endif // __cpp_rtti >= 199711L

} // namespace details

namespace skills {

#ifdef PRO4D_HAS_FORMAT
template <class FB>
using format =
    typename FB::template add_convention<details::format_dispatch,
                                         details::format_overload_t<char>>;

template <class FB>
using wformat =
    typename FB::template add_convention<details::format_dispatch,
                                         details::format_overload_t<wchar_t>>;
#endif // PRO4D_HAS_FORMAT

#if __cpp_rtti >= 199711L
template <class FB>
using indirect_rtti = typename FB::template add_indirect_convention<
    details::proxy_cast_dispatch, void(details::proxy_cast_context) &,
    void(details::proxy_cast_context) const&,
    void(details::proxy_cast_context) &&>::
    template add_indirect_reflection<details::proxy_typeid_reflector>;

template <class FB>
using direct_rtti = typename FB::template add_direct_convention<
    details::proxy_cast_dispatch, void(details::proxy_cast_context) &,
    void(details::proxy_cast_context) const&,
    void(details::proxy_cast_context) &&>::
    template add_direct_reflection<details::proxy_typeid_reflector>;

template <class FB>
using rtti = indirect_rtti<FB>;
#endif // __cpp_rtti >= 199711L

template <class FB>
using slim =
    typename FB::template restrict_layout<sizeof(void*), alignof(void*)>;

template <class FB>
using as_view = typename FB::template add_direct_convention<
    details::view_conversion_dispatch,
    facade_aware_overload_t<details::view_conversion_overload>>;

template <class FB>
using as_weak = typename FB::template add_direct_convention<
    details::weak_conversion_dispatch,
    facade_aware_overload_t<details::weak_conversion_overload>>;

} // namespace skills

// =============================================================================
// == Dispatch Extensions (operator_dispatch, weak_dispatch, etc.)            ==
// =============================================================================

namespace details {

template <std::size_t N>
struct sign {
  consteval sign(const char (&str)[N + 1]) {
    if (str[N] != '\0') {
      PROD_UNREACHABLE();
    }
    for (std::size_t i = 0; i < N; ++i) {
      value[i] = str[i];
    }
  }

  char value[N];
};
template <std::size_t N>
sign(const char (&str)[N]) -> sign<N - 1u>;

struct wildcard {
  wildcard() = delete;

  template <class T>
  [[noreturn]] operator T() const {
    PROD_UNREACHABLE();
  }
};

} // namespace details

template <details::sign Sign, bool Rhs = false>
struct operator_dispatch;

#define PROD_DEF_LHS_LEFT_OP_ACCESSOR(oq, pq, ne, ...)                         \
  template <class P, class D, class R>                                         \
  struct accessor<P, D, R() oq ne> {                                           \
    PRO4D_GEN_DEBUG_SYMBOL_FOR_MEM_ACCESSOR(__VA_ARGS__)                       \
    R __VA_ARGS__() oq ne {                                                    \
      return proxy_invoke<D, R() oq ne>(static_cast<P pq>(*this));             \
    }                                                                          \
  }
#define PROD_DEF_LHS_UNARY_OP_ACCESSOR(oq, pq, ne, ...)                        \
  template <class P, class D, class R>                                         \
  struct accessor<P, D, R() oq ne> {                                           \
    PRO4D_GEN_DEBUG_SYMBOL_FOR_MEM_ACCESSOR(__VA_ARGS__)                       \
    decltype(auto) __VA_ARGS__() oq ne {                                       \
      proxy_invoke<D, R() oq ne>(static_cast<P pq>(*this));                    \
      return static_cast<P pq>(*this);                                         \
    }                                                                          \
  };                                                                           \
  template <class P, class D, class R>                                         \
  struct accessor<P, D, R(int) oq ne> {                                        \
    PRO4D_GEN_DEBUG_SYMBOL_FOR_MEM_ACCESSOR(__VA_ARGS__)                       \
    R __VA_ARGS__(int) oq ne {                                                 \
      return proxy_invoke<D, R(int) oq ne>(static_cast<P pq>(*this), 0);       \
    }                                                                          \
  }
#define PROD_DEF_LHS_BINARY_OP_ACCESSOR PRO4D_DEF_MEM_ACCESSOR
#define PROD_DEF_LHS_ALL_OP_ACCESSOR PRO4D_DEF_MEM_ACCESSOR
#define PROD_LHS_LEFT_OP_DISPATCH_BODY_IMPL(...)                               \
  template <class T>                                                           \
  PRO4D_STATIC_CALL(decltype(auto), T&& self)                                  \
  PRO4D_DIRECT_FUNC_IMPL(__VA_ARGS__ std::forward<T>(self))
#define PROD_LHS_UNARY_OP_DISPATCH_BODY_IMPL(...)                              \
  template <class T>                                                           \
  PRO4D_STATIC_CALL(decltype(auto), T&& self)                                  \
  PRO4D_DIRECT_FUNC_IMPL(__VA_ARGS__ std::forward<T>(self)) template <class T> \
  PRO4D_STATIC_CALL(decltype(auto), T&& self, int)                             \
  PRO4D_DIRECT_FUNC_IMPL(std::forward<T>(self) __VA_ARGS__)
#define PROD_LHS_BINARY_OP_DISPATCH_BODY_IMPL(...)                             \
  template <class T, class Arg>                                                \
  PRO4D_STATIC_CALL(decltype(auto), T&& self, Arg&& arg)                       \
  PRO4D_DIRECT_FUNC_IMPL(std::forward<T>(self)                                 \
                             __VA_ARGS__ std::forward<Arg>(arg))
#define PROD_LHS_ALL_OP_DISPATCH_BODY_IMPL(...)                                \
  PROD_LHS_LEFT_OP_DISPATCH_BODY_IMPL(__VA_ARGS__)                             \
  PROD_LHS_BINARY_OP_DISPATCH_BODY_IMPL(__VA_ARGS__)
#define PROD_LHS_OP_DISPATCH_IMPL(type, ...)                                   \
  template <>                                                                  \
  struct operator_dispatch<#__VA_ARGS__, false> {                              \
    PROD_LHS_##type##_OP_DISPATCH_BODY_IMPL(__VA_ARGS__)                       \
        PRO4D_DEF_ACCESSOR_TEMPLATE(                                           \
            MEM, PROD_DEF_LHS_##type##_OP_ACCESSOR, operator __VA_ARGS__)      \
  };

#define PROD_DEF_RHS_OP_ACCESSOR(oq, pq, ne, ...)                              \
  template <class P, class D, class R, class Arg>                              \
  struct accessor<P, D, R(Arg) oq ne> {                                        \
    friend R operator __VA_ARGS__(Arg arg, P pq self) ne {                     \
      return proxy_invoke<D, R(Arg) oq ne>(static_cast<P pq>(self),            \
                                           std::forward<Arg>(arg));            \
    }                                                                          \
    PRO4D_DEBUG(                                                             \
      accessor() noexcept { std::ignore = &pro_symbol_guard; }               \
                                                                             \
    private:                                                                 \
      static inline R pro_symbol_guard(Arg arg, P pq self) {                 \
        return std::forward<Arg>(arg) __VA_ARGS__ static_cast<P pq>(self);   \
      }                                                                      \
    ) \
  }
#define PROD_RHS_OP_DISPATCH_IMPL(...)                                         \
  template <>                                                                  \
  struct operator_dispatch<#__VA_ARGS__, true> {                               \
    template <class T, class Arg>                                              \
    PRO4D_STATIC_CALL(decltype(auto), T&& self, Arg&& arg)                     \
    PRO4D_DIRECT_FUNC_IMPL(std::forward<Arg>(arg)                              \
                               __VA_ARGS__ std::forward<T>(self))              \
        PRO4D_DEF_ACCESSOR_TEMPLATE(FREE, PROD_DEF_RHS_OP_ACCESSOR,            \
                                    __VA_ARGS__)                               \
  };

#define PROD_EXTENDED_BINARY_OP_DISPATCH_IMPL(...)                             \
  PROD_LHS_OP_DISPATCH_IMPL(ALL, __VA_ARGS__)                                  \
  PROD_RHS_OP_DISPATCH_IMPL(__VA_ARGS__)

#define PROD_BINARY_OP_DISPATCH_IMPL(...)                                      \
  PROD_LHS_OP_DISPATCH_IMPL(BINARY, __VA_ARGS__)                               \
  PROD_RHS_OP_DISPATCH_IMPL(__VA_ARGS__)

#define PROD_DEF_LHS_ASSIGNMENT_OP_ACCESSOR(oq, pq, ne, ...)                   \
  template <class P, class D, class R, class Arg>                              \
  struct accessor<P, D, R(Arg) oq ne> {                                        \
    PRO4D_GEN_DEBUG_SYMBOL_FOR_MEM_ACCESSOR(__VA_ARGS__)                       \
    decltype(auto) __VA_ARGS__(Arg arg) oq ne {                                \
      proxy_invoke<D, R(Arg) oq ne>(static_cast<P pq>(*this),                  \
                                    std::forward<Arg>(arg));                   \
      return static_cast<P pq>(*this);                                         \
    }                                                                          \
  }
#define PROD_DEF_RHS_ASSIGNMENT_OP_ACCESSOR(oq, pq, ne, ...)                   \
  template <class P, class D, class R, class Arg>                              \
  struct accessor<P, D, R(Arg&) oq ne> {                                       \
    friend Arg& operator __VA_ARGS__(Arg& arg, P pq self) ne {                 \
      proxy_invoke<D, R(Arg&) oq ne>(static_cast<P pq>(self), arg);            \
      return arg;                                                              \
    }                                                                          \
    PRO4D_DEBUG(                                                               \
        accessor() noexcept { std::ignore = &pro_symbol_guard; }               \
                                                                               \
        private : static inline Arg& pro_symbol_guard(                         \
            Arg& arg,                                                          \
            P pq self) { return arg __VA_ARGS__ static_cast<P pq>(self); })    \
  }
#define PROD_ASSIGNMENT_OP_DISPATCH_IMPL(...)                                  \
  template <>                                                                  \
  struct operator_dispatch<#__VA_ARGS__, false> {                              \
    template <class T, class Arg>                                              \
    PRO4D_STATIC_CALL(decltype(auto), T&& self, Arg&& arg)                     \
    PRO4D_DIRECT_FUNC_IMPL(std::forward<T>(self)                               \
                               __VA_ARGS__ std::forward<Arg>(arg))             \
        PRO4D_DEF_ACCESSOR_TEMPLATE(                                           \
            MEM, PROD_DEF_LHS_ASSIGNMENT_OP_ACCESSOR, operator __VA_ARGS__)    \
  };                                                                           \
  template <>                                                                  \
  struct operator_dispatch<#__VA_ARGS__, true> {                               \
    template <class T, class Arg>                                              \
    PRO4D_STATIC_CALL(decltype(auto), T&& self, Arg&& arg)                     \
    PRO4D_DIRECT_FUNC_IMPL(std::forward<Arg>(arg)                              \
                               __VA_ARGS__ std::forward<T>(self))              \
        PRO4D_DEF_ACCESSOR_TEMPLATE(FREE, PROD_DEF_RHS_ASSIGNMENT_OP_ACCESSOR, \
                                    __VA_ARGS__)                               \
  };

PROD_EXTENDED_BINARY_OP_DISPATCH_IMPL(+)
PROD_EXTENDED_BINARY_OP_DISPATCH_IMPL(-)
PROD_EXTENDED_BINARY_OP_DISPATCH_IMPL(*)
PROD_BINARY_OP_DISPATCH_IMPL(/)
PROD_BINARY_OP_DISPATCH_IMPL(%)
PROD_LHS_OP_DISPATCH_IMPL(UNARY, ++)
PROD_LHS_OP_DISPATCH_IMPL(UNARY, --)
PROD_BINARY_OP_DISPATCH_IMPL(==)
PROD_BINARY_OP_DISPATCH_IMPL(!=)
PROD_BINARY_OP_DISPATCH_IMPL(>)
PROD_BINARY_OP_DISPATCH_IMPL(<)
PROD_BINARY_OP_DISPATCH_IMPL(>=)
PROD_BINARY_OP_DISPATCH_IMPL(<=)
PROD_BINARY_OP_DISPATCH_IMPL(<=>)
PROD_LHS_OP_DISPATCH_IMPL(LEFT, !)
PROD_BINARY_OP_DISPATCH_IMPL(&&)
PROD_BINARY_OP_DISPATCH_IMPL(||)
PROD_LHS_OP_DISPATCH_IMPL(LEFT, ~)
PROD_EXTENDED_BINARY_OP_DISPATCH_IMPL(&)
PROD_BINARY_OP_DISPATCH_IMPL(|)
PROD_BINARY_OP_DISPATCH_IMPL(^)
PROD_BINARY_OP_DISPATCH_IMPL(<<)
PROD_BINARY_OP_DISPATCH_IMPL(>>)
PROD_ASSIGNMENT_OP_DISPATCH_IMPL(+=)
PROD_ASSIGNMENT_OP_DISPATCH_IMPL(-=)
PROD_ASSIGNMENT_OP_DISPATCH_IMPL(*=)
PROD_ASSIGNMENT_OP_DISPATCH_IMPL(/=)
PROD_ASSIGNMENT_OP_DISPATCH_IMPL(%=)
PROD_ASSIGNMENT_OP_DISPATCH_IMPL(&=)
PROD_ASSIGNMENT_OP_DISPATCH_IMPL(|=)
PROD_ASSIGNMENT_OP_DISPATCH_IMPL(^=)
PROD_ASSIGNMENT_OP_DISPATCH_IMPL(<<=)
PROD_ASSIGNMENT_OP_DISPATCH_IMPL(>>=)
PROD_BINARY_OP_DISPATCH_IMPL(, )
PROD_BINARY_OP_DISPATCH_IMPL(->*)

template <>
struct operator_dispatch<"()", false> {
  template <class T, class... Args>
  PRO4D_STATIC_CALL(decltype(auto), T&& self, Args&&... args)
  PRO4D_DIRECT_FUNC_IMPL(std::forward<T>(self)(std::forward<Args>(args)...))
      PRO4D_DEF_ACCESSOR_TEMPLATE(MEM, PRO4D_DEF_MEM_ACCESSOR, operator())
};
template <>
struct operator_dispatch<"[]", false> {
#if __cpp_multidimensional_subscript >= 202110L
  template <class T, class... Args>
  PRO4D_STATIC_CALL(decltype(auto), T&& self, Args&&... args)
  PRO4D_DIRECT_FUNC_IMPL(std::forward<T>(self)[std::forward<Args>(args)...])
#else
  template <class T, class Arg>
  PRO4D_STATIC_CALL(decltype(auto), T&& self, Arg&& arg)
  PRO4D_DIRECT_FUNC_IMPL(std::forward<T>(self)[std::forward<Arg>(arg)])
#endif // __cpp_multidimensional_subscript >= 202110L
      PRO4D_DEF_ACCESSOR_TEMPLATE(MEM, PRO4D_DEF_MEM_ACCESSOR, operator[])
};

#undef PROD_ASSIGNMENT_OP_DISPATCH_IMPL
#undef PROD_DEF_RHS_ASSIGNMENT_OP_ACCESSOR
#undef PROD_DEF_LHS_ASSIGNMENT_OP_ACCESSOR
#undef PROD_BINARY_OP_DISPATCH_IMPL
#undef PROD_EXTENDED_BINARY_OP_DISPATCH_IMPL
#undef PROD_RHS_OP_DISPATCH_IMPL
#undef PROD_DEF_RHS_OP_ACCESSOR
#undef PROD_LHS_OP_DISPATCH_IMPL
#undef PROD_LHS_ALL_OP_DISPATCH_BODY_IMPL
#undef PROD_LHS_BINARY_OP_DISPATCH_BODY_IMPL
#undef PROD_LHS_UNARY_OP_DISPATCH_BODY_IMPL
#undef PROD_LHS_LEFT_OP_DISPATCH_BODY_IMPL
#undef PROD_DEF_LHS_ALL_OP_ACCESSOR
#undef PROD_DEF_LHS_BINARY_OP_ACCESSOR
#undef PROD_DEF_LHS_UNARY_OP_ACCESSOR
#undef PROD_DEF_LHS_LEFT_OP_ACCESSOR

struct implicit_conversion_dispatch
    : details::cast_dispatch_base<false, false> {
  template <class T>
  PRO4D_STATIC_CALL(T&&, T&& self) noexcept {
    return std::forward<T>(self);
  }
};
struct explicit_conversion_dispatch : details::cast_dispatch_base<true, false> {
  template <class T>
  PRO4D_STATIC_CALL(auto, T&& self) noexcept {
    return details::converter{
        [&self]<class U>(std::in_place_type_t<U>) noexcept(
            std::is_nothrow_constructible_v<U, T>)
          requires(std::is_constructible_v<U, T>)
        { return U{std::forward<T>(self)}; }};
  }
};
using conversion_dispatch = explicit_conversion_dispatch;

class not_implemented : public std::exception {
public:
  char const* what() const noexcept override {
    return "pro::v4::not_implemented";
  }
};

template <class D>
struct weak_dispatch : D {
  using D::operator();
  template <class... Args>
  [[noreturn]] PRO4D_STATIC_CALL(details::wildcard, Args&&...)
    requires(!std::is_invocable_v<D, Args...>)
  {
    PRO4D_THROW(not_implemented{});
  }
};

} // namespace pro::inline v4

// =============================================================================
// == Adapters (std::formatter)                                               ==
// =============================================================================

#ifdef PRO4D_HAS_FORMAT
namespace std {

template <pro::v4::facade F, class CharT>
  requires(pro::v4::details::facade_traits<F>::template is_invocable<
           false, pro::v4::details::format_dispatch,
           pro::v4::details::format_overload_t<CharT>>)
struct formatter<pro::v4::proxy_indirect_accessor<F>, CharT> {
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
  OutIt format(const pro::v4::proxy_indirect_accessor<F>& p,
               basic_format_context<OutIt, CharT>& fc) const {
    return pro::v4::proxy_invoke<pro::v4::details::format_dispatch,
                                 pro::v4::details::format_overload_t<CharT>>(
        p, spec_, fc);
  }

private:
  basic_string_view<CharT> spec_;
};

} // namespace std
#endif // PRO4D_HAS_FORMAT

#undef PROD_UNREACHABLE
#undef PROD_NO_UNIQUE_ADDRESS_ATTRIBUTE

#endif // MSFT_PROXY_V4_PROXY_H_
