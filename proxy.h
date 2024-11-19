// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#ifndef _MSFT_PROXY_
#define _MSFT_PROXY_

#include <cassert>
#include <cstddef>
#include <bit>
#include <concepts>
#include <initializer_list>
#include <limits>
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

#ifdef _MSC_VER
#define ___PRO_ENFORCE_EBO __declspec(empty_bases)
#else
#define ___PRO_ENFORCE_EBO
#endif  // _MSC_VER

#define __msft_lib_proxy 202410L

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

struct applicable_traits { static constexpr bool applicable = true; };
struct inapplicable_traits { static constexpr bool applicable = false; };

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

template <template <class, class> class R, class O, class... Is>
struct recursive_reduction : std::type_identity<O> {};
template <template <class, class> class R, class O, class... Is>
using recursive_reduction_t = typename recursive_reduction<R, O, Is...>::type;
template <template <class, class> class R, class O, class I, class... Is>
struct recursive_reduction<R, O, I, Is...>
    { using type = recursive_reduction_t<R, R<O, I>, Is...>; };

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

template <class P, qualifier_type Q, bool NE>
struct ptr_traits : inapplicable_traits {};
template <class P, qualifier_type Q, bool NE>
    requires(requires { *std::declval<add_qualifier_t<P, Q>>(); } &&
        (!NE || noexcept(*std::declval<add_qualifier_t<P, Q>>())))
struct ptr_traits<P, Q, NE> : applicable_traits
    { using target_type = decltype(*std::declval<add_qualifier_t<P, Q>>()); };

template <class D, bool NE, class R, class... Args>
concept invocable_dispatch = (NE && std::is_nothrow_invocable_r_v<
    R, D, Args...>) || (!NE && std::is_invocable_r_v<R, D, Args...>);
template <class D, class P, qualifier_type Q, bool NE, class R, class... Args>
concept invocable_dispatch_ptr_indirect = ptr_traits<P, Q, NE>::applicable &&
    invocable_dispatch<
        D, NE, R, typename ptr_traits<P, Q, NE>::target_type, Args...>;
template <class D, class P, qualifier_type Q, bool NE, class R, class... Args>
concept invocable_dispatch_ptr_direct = invocable_dispatch<
    D, NE, R, add_qualifier_t<P, Q>, Args...> && (Q != qualifier_type::rv ||
        (NE && std::is_nothrow_destructible_v<P>) ||
        (!NE && std::is_destructible_v<P>));

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
template <class D, class P, qualifier_type Q, class R, class... Args>
R indirect_conv_dispatcher(add_qualifier_t<std::byte, Q> self, Args... args)
    noexcept(invocable_dispatch_ptr_indirect<D, P, Q, true, R, Args...>) {
  return invoke_dispatch<D, R>(*std::forward<add_qualifier_t<P, Q>>(
      *std::launder(reinterpret_cast<add_qualifier_ptr_t<P, Q>>(&self))),
      std::forward<Args>(args)...);
}
template <class D, class P, qualifier_type Q, class R, class... Args>
R direct_conv_dispatcher(add_qualifier_t<std::byte, Q> self, Args... args)
    noexcept(invocable_dispatch_ptr_direct<D, P, Q, true, R, Args...>) {
  auto& qp = *std::launder(
      reinterpret_cast<add_qualifier_ptr_t<P, Q>>(&self));
  if constexpr (Q == qualifier_type::rv) {
    destruction_guard guard{&qp};
    return invoke_dispatch<D, R>(
        std::forward<add_qualifier_t<P, Q>>(qp), std::forward<Args>(args)...);
  } else {
    return invoke_dispatch<D, R>(
        std::forward<add_qualifier_t<P, Q>>(qp), std::forward<Args>(args)...);
  }
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
  destruction_guard guard{other};
  std::construct_at(reinterpret_cast<P*>(&self), std::move(*other));
}
template <class P>
void destruction_dispatcher(std::byte& self)
    noexcept(has_destructibility<P>(constraint_level::nothrow))
    { std::destroy_at(std::launder(reinterpret_cast<P*>(&self))); }
inline void destruction_default_dispatcher(std::byte&) noexcept {}

template <class O> struct overload_traits : inapplicable_traits {};
template <qualifier_type Q, bool NE, class R, class... Args>
struct overload_traits_impl : applicable_traits {
  template <bool IS_DIRECT, class D>
  struct meta_provider {
    template <class P>
    static constexpr auto get()
        -> func_ptr_t<NE, R, add_qualifier_t<std::byte, Q>, Args...> {
      if constexpr (!IS_DIRECT &&
          invocable_dispatch_ptr_indirect<D, P, Q, NE, R, Args...>) {
        return &indirect_conv_dispatcher<D, P, Q, R, Args...>;
      } else if constexpr (IS_DIRECT &&
          invocable_dispatch_ptr_direct<D, P, Q, NE, R, Args...>) {
        return &direct_conv_dispatcher<D, P, Q, R, Args...>;
      } else if constexpr (invocable_dispatch<
          D, NE, R, std::nullptr_t, Args...>) {
        return &default_conv_dispatcher<D, Q, R, Args...>;
      } else {
        return nullptr;
      }
    }
  };

  template <bool IS_DIRECT, class D, class P>
  static constexpr bool applicable_ptr =
      meta_provider<IS_DIRECT, D>::template get<P>() != nullptr;
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

template <class MP>
struct dispatcher_meta {
  constexpr dispatcher_meta() noexcept : dispatcher(nullptr) {}
  template <class P>
  constexpr explicit dispatcher_meta(std::in_place_type_t<P>) noexcept
      : dispatcher(MP::template get<P>()) {}

  decltype(MP::template get<void>()) dispatcher;
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
struct conv_traits_impl : inapplicable_traits {};
template <class C, class... Os>
    requires(sizeof...(Os) > 0u && (overload_traits<Os>::applicable && ...))
struct conv_traits_impl<C, Os...> : applicable_traits {
  using meta = composite_meta_impl<dispatcher_meta<typename overload_traits<Os>
      ::template meta_provider<C::is_direct, typename C::dispatch_type>>...>;

  template <class P>
  static constexpr bool applicable_ptr =
      (overload_traits<Os>::template applicable_ptr<
          C::is_direct, typename C::dispatch_type, P> && ...);
};
template <class C> struct conv_traits : inapplicable_traits {};
template <class C>
    requires(
        requires {
          typename C::dispatch_type;
          typename C::overload_types;
        } &&
        is_is_direct_well_formed<C>() &&
        std::is_trivial_v<typename C::dispatch_type> &&
        is_tuple_like_well_formed<typename C::overload_types>())
struct conv_traits<C>
    : instantiated_t<conv_traits_impl, typename C::overload_types, C> {};

template <class P>
using ptr_element_t = typename std::pointer_traits<P>::element_type;
template <class R>
struct refl_meta {
  template <class P> requires(R::is_direct)
  constexpr explicit refl_meta(std::in_place_type_t<P>)
      : reflector(std::in_place_type<P>) {}
  template <class P> requires(!R::is_direct)
  constexpr explicit refl_meta(std::in_place_type_t<P>)
      : reflector(std::in_place_type<ptr_element_t<P>>) {}

  typename R::reflector_type reflector;
};

template <class R, class T, bool IS_DIRECT>
consteval bool is_reflector_well_formed() {
  if constexpr (IS_DIRECT) {
    if constexpr (std::is_constructible_v<R, std::in_place_type_t<T>>) {
      if constexpr (is_consteval([] { return R{std::in_place_type<T>}; })) {
        return true;
      }
    }
  } else if constexpr (requires { typename ptr_element_t<T>; }) {
    return is_reflector_well_formed<R, ptr_element_t<T>, true>();
  }
  return false;
}
template <class R> struct refl_traits : inapplicable_traits {};
template <class R>
    requires(requires { typename R::reflector_type; } &&
        is_is_direct_well_formed<R>())
struct refl_traits<R> : applicable_traits {
  template <class P>
  static constexpr bool applicable_ptr =
      is_reflector_well_formed<typename R::reflector_type, P, R::is_direct>();
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

template <class F> struct proxy_indirect_accessor;
template <class... As>
class ___PRO_ENFORCE_EBO composite_accessor_impl : public As... {
  template <class> friend class pro::proxy;
  template <class F> friend struct proxy_indirect_accessor;

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
template <class SFINAE, class T, class... Args>
struct sfinae_accessor_traits : std::type_identity<void> {};
template <class T, class... Args>
struct sfinae_accessor_traits<
    std::void_t<typename T::template accessor<Args...>>, T, Args...>
    : accessor_traits_impl<typename T::template accessor<Args...>> {};
template <class T, class... Args>
using accessor_t = typename sfinae_accessor_traits<void, T, Args...>::type;

template <bool IS_DIRECT, class F, class O, class I>
struct composite_accessor_reduction : std::type_identity<O> {};
template <bool IS_DIRECT, class F, class... As, class I>
    requires(IS_DIRECT == I::is_direct && !std::is_void_v<accessor_t<I, F>>)
struct composite_accessor_reduction<
    IS_DIRECT, F, composite_accessor_impl<As...>, I>
    { using type = composite_accessor_impl<As..., accessor_t<I, F>>; };
template <bool IS_DIRECT, class F>
struct composite_accessor_helper {
  template <class O, class I>
  using reduction_t =
      typename composite_accessor_reduction<IS_DIRECT, F, O, I>::type;
};
template <bool IS_DIRECT, class F, class... Ts>
using composite_accessor = recursive_reduction_t<
      composite_accessor_helper<IS_DIRECT, F>::template reduction_t,
      composite_accessor_impl<>, Ts...>;

template <class A1, class A2> struct composite_accessor_merge_traits;
template <class... A1, class... A2>
struct composite_accessor_merge_traits<
    composite_accessor_impl<A1...>, composite_accessor_impl<A2...>>
    : std::type_identity<composite_accessor_impl<A1..., A2...>> {};
template <class A1, class A2>
using merged_composite_accessor =
    typename composite_accessor_merge_traits<A1, A2>::type;

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
template <class F, class... Cs>
struct facade_conv_traits_impl : inapplicable_traits {};
template <class F, class... Cs> requires(conv_traits<Cs>::applicable && ...)
struct facade_conv_traits_impl<F, Cs...> : applicable_traits {
  using conv_meta = composite_meta<typename conv_traits<Cs>::meta...>;
  using conv_indirect_accessor = composite_accessor<false, F, Cs...>;
  using conv_direct_accessor = composite_accessor<true, F, Cs...>;

  template <class P>
  static constexpr bool conv_applicable_ptr =
      (conv_traits<Cs>::template applicable_ptr<P> && ...);
};
template <class F, class... Rs>
struct facade_refl_traits_impl {
  using refl_meta = composite_meta<details::refl_meta<Rs>...>;
  using refl_indirect_accessor = composite_accessor<false, F, Rs...>;
  using refl_direct_accessor = composite_accessor<true, F, Rs...>;

  template <class P>
  static constexpr bool refl_applicable_ptr =
      (refl_traits<Rs>::template applicable_ptr<P> && ...);
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
        is_tuple_like_well_formed<typename F::reflection_types>())
struct facade_traits<F>
    : instantiated_t<facade_conv_traits_impl, typename F::convention_types, F>,
      instantiated_t<facade_refl_traits_impl, typename F::reflection_types, F> {
  using copyability_meta = lifetime_meta_t<
      copyability_meta_provider, F::constraints.copyability>;
  using relocatability_meta = lifetime_meta_t<
      relocatability_meta_provider,
      F::constraints.copyability == constraint_level::trivial ?
          constraint_level::trivial : F::constraints.relocatability>;
  using destructibility_meta = lifetime_meta_t<
      destructibility_meta_provider, F::constraints.destructibility>;
  using meta = composite_meta<copyability_meta, relocatability_meta,
      destructibility_meta, typename facade_traits::conv_meta,
      typename facade_traits::refl_meta>;
  using indirect_accessor = merged_composite_accessor<
      typename facade_traits::conv_indirect_accessor,
      typename facade_traits::refl_indirect_accessor>;
  using direct_accessor = merged_composite_accessor<
      typename facade_traits::conv_direct_accessor,
      typename facade_traits::refl_direct_accessor>;
  static constexpr bool has_indirection = !std::is_same_v<
      typename facade_traits::indirect_accessor, composite_accessor_impl<>>;
};

template <class F> struct proxy_indirect_accessor {};
template <class F> requires(facade_traits<F>::has_indirection)
struct proxy_indirect_accessor<F> : facade_traits<F>::indirect_accessor {};

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
template <class MP, class... Ms>
struct meta_ptr_traits_impl<composite_meta_impl<dispatcher_meta<MP>, Ms...>>
    : std::type_identity<meta_ptr_direct_impl<composite_meta_impl<
          dispatcher_meta<MP>, Ms...>, dispatcher_meta<MP>>> {};
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
  template <class C, class O, qualifier_type Q, class... Args>
  static decltype(auto) invoke(add_qualifier_t<proxy<F>, Q> p, Args&&... args) {
    auto dispatcher = get_meta(p)
        .template dispatcher_meta<typename overload_traits<O>
        ::template meta_provider<C::is_direct, typename C::dispatch_type>>
        ::dispatcher;
    if constexpr (C::is_direct &&
        overload_traits<O>::qualifier == qualifier_type::rv) {
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
      return reinterpret_cast<add_qualifier_t<proxy<F>, Q>>(
          *(reinterpret_cast<add_qualifier_ptr_t<std::byte, Q>>(
              static_cast<add_qualifier_ptr_t<
                  typename facade_traits<F>::indirect_accessor, Q>>(
                      std::addressof(a))) - offsetof(proxy<F>, ia_)));
    }
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
    details::facade_traits<F>::template conv_applicable_ptr<P> &&
    details::facade_traits<F>::template refl_applicable_ptr<P>;

template <class F>
class proxy : public details::facade_traits<F>::direct_accessor {
  static_assert(facade<F>);
  friend struct details::proxy_helper<F>;
  using _Traits = details::facade_traits<F>;
  using _IA = details::proxy_indirect_accessor<F>;

 public:
#ifdef NDEBUG
  proxy() noexcept = default;
#else
  proxy() noexcept {
    if constexpr (_Traits::has_indirection) {
      std::ignore = static_cast<_IA* (proxy::*)() noexcept>(&proxy::operator->);
      std::ignore = static_cast<const _IA* (proxy::*)() const noexcept>(
          &proxy::operator->);
      std::ignore = static_cast<_IA& (proxy::*)() & noexcept>(&proxy::operator*);
      std::ignore = static_cast<const _IA& (proxy::*)() const& noexcept>(
          &proxy::operator*);
      std::ignore = static_cast<_IA&& (proxy::*)() && noexcept>(
          &proxy::operator*);
      std::ignore = static_cast<const _IA&& (proxy::*)() const&& noexcept>(
          &proxy::operator*);
    }
  }
#endif  // NDEBUG
  proxy(std::nullptr_t) noexcept : proxy() {}
  proxy(const proxy&) noexcept requires(F::constraints.copyability ==
      constraint_level::trivial) = default;
  proxy(const proxy& rhs)
      noexcept(F::constraints.copyability == constraint_level::nothrow)
      requires(F::constraints.copyability == constraint_level::nontrivial ||
          F::constraints.copyability == constraint_level::nothrow) {
    if (rhs.meta_.has_value()) {
      rhs.meta_->_Traits::copyability_meta::dispatcher(*ptr_, *rhs.ptr_);
      meta_ = rhs.meta_;
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
      } else {
        rhs.meta_->_Traits::relocatability_meta::dispatcher(*ptr_, *rhs.ptr_);
      }
      meta_ = rhs.meta_;
    }
  }
  template <class P>
  proxy(P&& ptr) noexcept(std::is_nothrow_constructible_v<std::decay_t<P>, P>)
      requires(proxiable<std::decay_t<P>, F> &&
          std::is_constructible_v<std::decay_t<P>, P>) : proxy()
      { initialize<std::decay_t<P>>(std::forward<P>(ptr)); }
  template <proxiable<F> P, class... Args>
  explicit proxy(std::in_place_type_t<P>, Args&&... args)
      noexcept(std::is_nothrow_constructible_v<P, Args...>)
      requires(std::is_constructible_v<P, Args...>) : proxy()
      { initialize<P>(std::forward<Args>(args)...); }
  template <proxiable<F> P, class U, class... Args>
  explicit proxy(std::in_place_type_t<P>, std::initializer_list<U> il,
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
  proxy& operator=(P&& ptr)
      noexcept(std::is_nothrow_constructible_v<std::decay_t<P>, P> &&
          F::constraints.destructibility >= constraint_level::nothrow)
      requires(proxiable<std::decay_t<P>, F> &&
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
      meta_->_Traits::destructibility_meta::dispatcher(*ptr_);
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
  template <proxiable<F> P, class... Args>
  P& emplace(Args&&... args)
      noexcept(std::is_nothrow_constructible_v<P, Args...> &&
          F::constraints.destructibility >= constraint_level::nothrow)
      requires(std::is_constructible_v<P, Args...> &&
          F::constraints.destructibility >= constraint_level::nontrivial)
      { reset(); return initialize<P>(std::forward<Args>(args)...); }
  template <proxiable<F> P, class U, class... Args>
  P& emplace(std::initializer_list<U> il, Args&&... args)
      noexcept(std::is_nothrow_constructible_v<
          P, std::initializer_list<U>&, Args...> &&
          F::constraints.destructibility >= constraint_level::nothrow)
      requires(std::is_constructible_v<P, std::initializer_list<U>&, Args...> &&
          F::constraints.destructibility >= constraint_level::nontrivial)
      { reset(); return initialize<P>(il, std::forward<Args>(args)...); }
  _IA* operator->() noexcept requires(_Traits::has_indirection)
      { return std::addressof(ia_); }
  const _IA* operator->() const noexcept requires(_Traits::has_indirection)
      { return std::addressof(ia_); }
  _IA& operator*() & noexcept requires(_Traits::has_indirection)
      { return ia_; }
  const _IA& operator*() const& noexcept requires(_Traits::has_indirection)
      { return ia_; }
  _IA&& operator*() && noexcept requires(_Traits::has_indirection)
      { return std::forward<_IA>(ia_); }
  const _IA&& operator*() const&& noexcept requires(_Traits::has_indirection)
      { return std::forward<const _IA>(ia_); }

  friend void swap(proxy& lhs, proxy& rhs) noexcept(noexcept(lhs.swap(rhs)))
      { lhs.swap(rhs); }
  friend bool operator==(const proxy& lhs, std::nullptr_t) noexcept
      { return !lhs.has_value(); }

 private:
  template <class P, class... Args>
  P& initialize(Args&&... args) {
    P& result = *std::construct_at(
        reinterpret_cast<P*>(ptr_), std::forward<Args>(args)...);
    if constexpr (requires { (bool)result; })
        { assert((bool)result); }
    meta_ = details::meta_ptr<typename _Traits::meta>{std::in_place_type<P>};
    return result;
  }

  [[___PRO_NO_UNIQUE_ADDRESS_ATTRIBUTE]]
  _IA ia_;
  details::meta_ptr<typename _Traits::meta> meta_;
  alignas(F::constraints.max_align) std::byte ptr_[F::constraints.max_size];
};

template <class C, class O, class F, class... Args>
decltype(auto) proxy_invoke(proxy<F>& p, Args&&... args) {
  return details::proxy_helper<F>::template invoke<
      C, O, details::qualifier_type::lv>(p, std::forward<Args>(args)...);
}
template <class C, class O, class F, class... Args>
decltype(auto) proxy_invoke(const proxy<F>& p, Args&&... args) {
  return details::proxy_helper<F>::template invoke<
      C, O, details::qualifier_type::const_lv>(p, std::forward<Args>(args)...);
}
template <class C, class O, class F, class... Args>
decltype(auto) proxy_invoke(proxy<F>&& p, Args&&... args) {
  return details::proxy_helper<F>::template invoke<
      C, O, details::qualifier_type::rv>(
      std::forward<proxy<F>>(p), std::forward<Args>(args)...);
}
template <class C, class O, class F, class... Args>
decltype(auto) proxy_invoke(const proxy<F>&& p, Args&&... args) {
  return details::proxy_helper<F>::template invoke<
      C, O, details::qualifier_type::const_rv>(
      std::forward<const proxy<F>>(p), std::forward<Args>(args)...);
}

template <class R, class F>
const auto& proxy_reflect(const proxy<F>& p) noexcept {
  return static_cast<const details::refl_meta<R>&>(
      details::proxy_helper<F>::get_meta(p)).reflector;
}

template <class F, class A>
proxy<F>& access_proxy(A& a) noexcept {
  return details::proxy_helper<F>::template access<
      A, details::qualifier_type::lv>(a);
}
template <class F, class A>
const proxy<F>& access_proxy(const A& a) noexcept {
  return details::proxy_helper<F>::template access<
      A, details::qualifier_type::const_lv>(a);
}
template <class F, class A>
proxy<F>&& access_proxy(A&& a) noexcept {
  return details::proxy_helper<F>::template access<
      A, details::qualifier_type::rv>(std::forward<A>(a));
}
template <class F, class A>
const proxy<F>&& access_proxy(const A&& a) noexcept {
  return details::proxy_helper<F>::template access<
      A, details::qualifier_type::const_rv>(std::forward<const A>(a));
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

  T* operator->() noexcept { return &value_; }
  const T* operator->() const noexcept { return &value_; }
  T& operator*() & noexcept { return value_; }
  const T& operator*() const& noexcept { return value_; }
  T&& operator*() && noexcept { return std::forward<T>(value_); }
  const T&& operator*() const&& noexcept
      { return std::forward<const T>(value_); }

 private:
  T value_;
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

  T* operator->() noexcept { return ptr_; }
  const T* operator->() const noexcept { return ptr_; }
  T& operator*() & noexcept { return *ptr_; }
  const T& operator*() const& noexcept { return *ptr_; }
  T&& operator*() && noexcept { return std::forward<T>(*ptr_); }
  const T&& operator*() const&& noexcept
      { return std::forward<const T>(*ptr_); }

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

  T* operator->() noexcept { return &ptr_->value; }
  const T* operator->() const noexcept { return &ptr_->value; }
  T& operator*() & noexcept { return ptr_->value; }
  const T& operator*() const& noexcept { return ptr_->value; }
  T&& operator*() && noexcept { return std::forward<T>(ptr_->value); }
  const T&& operator*() const&& noexcept
      { return std::forward<const T>(ptr_->value); }

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

#define ___PRO_DIRECT_FUNC_IMPL(...) \
    noexcept(noexcept(__VA_ARGS__)) requires(requires { __VA_ARGS__; }) \
    { return __VA_ARGS__; }

#define ___PRO_DEF_MEM_ACCESSOR_TEMPLATE(__MACRO, ...) \
    template <class __F, class __C, class... __Os> \
    struct ___PRO_ENFORCE_EBO accessor { accessor() = delete; }; \
    template <class __F, class __C, class... __Os> \
        requires(sizeof...(__Os) > 1u && \
            (::std::is_constructible_v<accessor<__F, __C, __Os>> && ...)) \
    struct accessor<__F, __C, __Os...> : accessor<__F, __C, __Os>... \
        { using accessor<__F, __C, __Os>::__VA_ARGS__...; }; \
    __MACRO(, *this, __VA_ARGS__); \
    __MACRO(noexcept, *this, __VA_ARGS__); \
    __MACRO(&, *this, __VA_ARGS__); \
    __MACRO(& noexcept, *this, __VA_ARGS__); \
    __MACRO(&&, ::std::forward<accessor>(*this), __VA_ARGS__); \
    __MACRO(&& noexcept, ::std::forward<accessor>(*this), __VA_ARGS__); \
    __MACRO(const, *this, __VA_ARGS__); \
    __MACRO(const noexcept, *this, __VA_ARGS__); \
    __MACRO(const&, *this, __VA_ARGS__); \
    __MACRO(const& noexcept, *this, __VA_ARGS__); \
    __MACRO(const&&, ::std::forward<const accessor>(*this), __VA_ARGS__); \
    __MACRO(const&& noexcept, ::std::forward<const accessor>(*this), \
        __VA_ARGS__);

#define ___PRO_DEF_FREE_ACCESSOR_TEMPLATE(__MACRO, ...) \
    template <class __F, class __C, class... __Os> \
    struct ___PRO_ENFORCE_EBO accessor { accessor() = delete; }; \
    template <class __F, class __C, class... __Os> \
        requires(sizeof...(__Os) > 1u && \
            (::std::is_constructible_v<accessor<__F, __C, __Os>> && ...)) \
    struct accessor<__F, __C, __Os...> : accessor<__F, __C, __Os>... {}; \
    __MACRO(,, accessor& __self, __self, __VA_ARGS__); \
    __MACRO(noexcept, noexcept, accessor& __self, __self, __VA_ARGS__); \
    __MACRO(&,, accessor& __self, __self, __VA_ARGS__); \
    __MACRO(& noexcept, noexcept, accessor& __self, __self, __VA_ARGS__); \
    __MACRO(&&,, accessor&& __self, \
        ::std::forward<accessor>(__self), __VA_ARGS__); \
    __MACRO(&& noexcept, noexcept, accessor&& __self, \
        ::std::forward<accessor>(__self), __VA_ARGS__); \
    __MACRO(const,, const accessor& __self, __self, __VA_ARGS__); \
    __MACRO(const noexcept, noexcept, const accessor& __self, \
        __self, __VA_ARGS__); \
    __MACRO(const&,, const accessor& __self, __self, __VA_ARGS__); \
    __MACRO(const& noexcept, noexcept, const accessor& __self, \
        __self, __VA_ARGS__); \
    __MACRO(const&&,, const accessor&& __self, \
        ::std::forward<const accessor>(__self), __VA_ARGS__); \
    __MACRO(const&& noexcept, noexcept, const accessor&& __self, \
        ::std::forward<const accessor>(__self), __VA_ARGS__);

#ifdef NDEBUG
#define ___PRO_GEN_SYMBOL_FOR_MEM_ACCESSOR(...)
#else
#define ___PRO_GEN_SYMBOL_FOR_MEM_ACCESSOR(...) \
    accessor() noexcept { ::std::ignore = &accessor::__VA_ARGS__; }
#endif  // NDEBUG

namespace details {

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

template <bool IS_DIRECT, class D, class... Os>
struct conv_impl {
  static constexpr bool is_direct = IS_DIRECT;
  using dispatch_type = D;
  using overload_types = std::tuple<Os...>;
  template <class F>
  using accessor = accessor_t<D, F, conv_impl, Os...>;
};
template <bool IS_DIRECT, class R>
struct refl_impl {
  static constexpr bool is_direct = IS_DIRECT;
  using reflector_type = R;
  template <class F>
  using accessor = accessor_t<R, F, refl_impl>;
};
template <class Cs, class Rs, proxiable_ptr_constraints C>
struct facade_impl {
  using convention_types = Cs;
  using reflection_types = Rs;
  static constexpr proxiable_ptr_constraints constraints = C;
};

#define ___PRO_DEF_UPWARD_CONVERSION_ACCESSOR(Q, SELF, ...) \
    template <class F2, class C> \
    struct accessor<F2, C, proxy<F>() Q> { \
      ___PRO_GEN_SYMBOL_FOR_MEM_ACCESSOR(__VA_ARGS__) \
      __VA_ARGS__ () Q { \
        if (access_proxy<F2>(SELF).has_value()) { \
          return proxy_invoke<C, proxy<F>() Q>(access_proxy<F2>(SELF)); \
        } \
        return nullptr; \
      } \
    }
template <class F>
struct upward_conversion_dispatch {
  template <class T>
  proxy<F> operator()(T&& value)
      noexcept(std::is_nothrow_convertible_v<T, proxy<F>>)
      requires(std::is_convertible_v<T, proxy<F>>)
      { return static_cast<proxy<F>>(std::forward<T>(value)); }
  ___PRO_DEF_MEM_ACCESSOR_TEMPLATE(
      ___PRO_DEF_UPWARD_CONVERSION_ACCESSOR, operator proxy<F>)
};
#undef ___PRO_DEF_UPWARD_CONVERSION_ACCESSOR

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

template <bool IS_DIRECT, class D>
struct merge_conv_traits
    { template <class... Os> using type = conv_impl<IS_DIRECT, D, Os...>; };
template <class C0, class C1>
using merge_conv_t = instantiated_t<
    merge_conv_traits<C0::is_direct, typename C0::dispatch_type>::template type,
    merge_tuple_t<typename C0::overload_types, typename C1::overload_types>>;

template <class Cs0, class C1, class C> struct add_conv_reduction;
template <class... Cs0, class C1, class... Cs2, class C>
struct add_conv_reduction<std::tuple<Cs0...>, std::tuple<C1, Cs2...>, C>
    : add_conv_reduction<std::tuple<Cs0..., C1>, std::tuple<Cs2...>, C> {};
template <class... Cs0, class C1, class... Cs2, class C>
    requires(C::is_direct == C1::is_direct && std::is_same_v<
        typename C::dispatch_type, typename C1::dispatch_type>)
struct add_conv_reduction<std::tuple<Cs0...>, std::tuple<C1, Cs2...>, C>
    : std::type_identity<std::tuple<Cs0..., merge_conv_t<C1, C>, Cs2...>> {};
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
          upward_conversion_dispatch<F>, copy_conversion_overload<F, CCL>,
          move_conversion_overload<F, RCL>>>> {};
template <class Cs, class F, constraint_level RCL>
struct add_upward_conversion_conv<Cs, F, constraint_level::none, RCL>
    : std::type_identity<add_conv_t<Cs, conv_impl<true,
          upward_conversion_dispatch<F>, move_conversion_overload<F, RCL>>>> {};
template <class Cs, class F, constraint_level CCL>
struct add_upward_conversion_conv<Cs, F, CCL, constraint_level::none>
    : std::type_identity<add_conv_t<Cs, conv_impl<true,
          upward_conversion_dispatch<F>, copy_conversion_overload<F, CCL>>>> {};
template <class Cs, class F>
struct add_upward_conversion_conv<
    Cs, F, constraint_level::none, constraint_level::none>
    : std::type_identity<Cs> {};

template <class Cs0, class... Cs1>
using merge_conv_tuple_t = recursive_reduction_t<add_conv_t, Cs0, Cs1...>;
template <class Cs, class F, bool WithUpwardConversion>
using merge_facade_conv_t = typename add_upward_conversion_conv<
    instantiated_t<merge_conv_tuple_t, typename F::convention_types, Cs>, F,
    WithUpwardConversion ? F::constraints.copyability : constraint_level::none,
    (WithUpwardConversion &&
        F::constraints.copyability != constraint_level::trivial) ?
        F::constraints.relocatability : constraint_level::none>::type;

template <std::size_t N>
struct sign {
  consteval sign(const char (&str)[N])
      { for (std::size_t i = 0; i < N; ++i) { value[i] = str[i]; } }

  char value[N];
};
template <std::size_t N>
sign(const char (&str)[N]) -> sign<N>;

}  // namespace details

template <class Cs, class Rs, proxiable_ptr_constraints C>
struct basic_facade_builder {
  template <class D, class... Os>
      requires(sizeof...(Os) > 0u &&
          (details::overload_traits<Os>::applicable && ...))
  using add_indirect_convention = basic_facade_builder<details::add_conv_t<
      Cs, details::conv_impl<false, D, Os...>>, Rs, C>;
  template <class D, class... Os>
      requires(sizeof...(Os) > 0u &&
          (details::overload_traits<Os>::applicable && ...))
  using add_direct_convention = basic_facade_builder<details::add_conv_t<
      Cs, details::conv_impl<true, D, Os...>>, Rs, C>;
  template <class D, class... Os>
      requires(sizeof...(Os) > 0u &&
          (details::overload_traits<Os>::applicable && ...))
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

template <details::sign Sign, bool Rhs = false>
struct operator_dispatch;

#define ___PRO_DEF_LHS_LEFT_OP_ACCESSOR(Q, SELF, ...) \
    template <class F, class C, class R> \
    struct accessor<F, C, R() Q> { \
      ___PRO_GEN_SYMBOL_FOR_MEM_ACCESSOR(__VA_ARGS__) \
      R __VA_ARGS__ () Q \
          { return proxy_invoke<C, R() Q>(access_proxy<F>(SELF)); } \
    }
#define ___PRO_DEF_LHS_ANY_OP_ACCESSOR(Q, SELF, ...) \
    template <class F, class C, class R, class... Args> \
    struct accessor<F, C, R(Args...) Q> { \
      ___PRO_GEN_SYMBOL_FOR_MEM_ACCESSOR(__VA_ARGS__) \
      R __VA_ARGS__ (Args... args) Q { \
        return proxy_invoke<C, R(Args...) Q>( \
            access_proxy<F>(SELF), std::forward<Args>(args)...); \
      } \
    }
#define ___PRO_DEF_LHS_UNARY_OP_ACCESSOR ___PRO_DEF_LHS_ANY_OP_ACCESSOR
#define ___PRO_DEF_LHS_BINARY_OP_ACCESSOR ___PRO_DEF_LHS_ANY_OP_ACCESSOR
#define ___PRO_DEF_LHS_ALL_OP_ACCESSOR ___PRO_DEF_LHS_ANY_OP_ACCESSOR
#define ___PRO_LHS_LEFT_OP_DISPATCH_BODY_IMPL(...) \
    template <class T> \
    decltype(auto) operator()(T&& self) \
        ___PRO_DIRECT_FUNC_IMPL(__VA_ARGS__ std::forward<T>(self))
#define ___PRO_LHS_UNARY_OP_DISPATCH_BODY_IMPL(...) \
    template <class T> \
    decltype(auto) operator()(T&& self) \
        ___PRO_DIRECT_FUNC_IMPL(__VA_ARGS__ std::forward<T>(self)) \
    template <class T> \
    decltype(auto) operator()(T&& self, int) \
        ___PRO_DIRECT_FUNC_IMPL(std::forward<T>(self) __VA_ARGS__)
#define ___PRO_LHS_BINARY_OP_DISPATCH_BODY_IMPL(...) \
    template <class T, class Arg> \
    decltype(auto) operator()(T&& self, Arg&& arg) \
        ___PRO_DIRECT_FUNC_IMPL(std::forward<T>(self) __VA_ARGS__ \
            std::forward<Arg>(arg))
#define ___PRO_LHS_ALL_OP_DISPATCH_BODY_IMPL(...) \
    ___PRO_LHS_LEFT_OP_DISPATCH_BODY_IMPL(__VA_ARGS__) \
    ___PRO_LHS_BINARY_OP_DISPATCH_BODY_IMPL(__VA_ARGS__)
#define ___PRO_LHS_OP_DISPATCH_IMPL(TYPE, ...) \
    template <> \
    struct operator_dispatch<#__VA_ARGS__, false> { \
      ___PRO_LHS_##TYPE##_OP_DISPATCH_BODY_IMPL(__VA_ARGS__) \
      ___PRO_DEF_MEM_ACCESSOR_TEMPLATE(___PRO_DEF_LHS_##TYPE##_OP_ACCESSOR, \
          operator __VA_ARGS__) \
    };

#ifdef NDEBUG
#define ___PRO_DEF_RHS_OP_ACCESSOR(Q, NE, SELF, FW_SELF, ...) \
    template <class F, class C, class R, class Arg> \
    struct accessor<F, C, R(Arg) Q> { \
      friend R operator __VA_ARGS__ (Arg arg, SELF) NE { \
        return proxy_invoke<C, R(Arg) Q>( \
            access_proxy<F>(FW_SELF), std::forward<Arg>(arg)); \
      } \
    }
#else
#define ___PRO_DEF_RHS_OP_ACCESSOR(Q, NE, SELF, FW_SELF, ...) \
    template <class F, class C, class R, class Arg> \
    struct accessor<F, C, R(Arg) Q> { \
      accessor() noexcept { std::ignore = &accessor::_symbol_guard; } \
      friend R operator __VA_ARGS__ (Arg arg, SELF) NE { \
        return proxy_invoke<C, R(Arg) Q>( \
            access_proxy<F>(FW_SELF), std::forward<Arg>(arg)); \
      } \
    \
     private: \
      static inline R _symbol_guard(Arg arg, SELF) NE \
          { return std::forward<Arg>(arg) __VA_ARGS__ FW_SELF; } \
    }
#endif  // NDEBUG
#define ___PRO_RHS_OP_DISPATCH_IMPL(...) \
    template <> \
    struct operator_dispatch<#__VA_ARGS__, true> { \
      template <class T, class Arg> \
      decltype(auto) operator()(T&& self, Arg&& arg) \
          ___PRO_DIRECT_FUNC_IMPL(std::forward<Arg>(arg) __VA_ARGS__ \
              std::forward<T>(self)) \
      ___PRO_DEF_FREE_ACCESSOR_TEMPLATE(___PRO_DEF_RHS_OP_ACCESSOR, \
          __VA_ARGS__) \
    };

#define ___PRO_EXTENDED_BINARY_OP_DISPATCH_IMPL(...) \
    ___PRO_LHS_OP_DISPATCH_IMPL(ALL, __VA_ARGS__) \
    ___PRO_RHS_OP_DISPATCH_IMPL(__VA_ARGS__)

#define ___PRO_BINARY_OP_DISPATCH_IMPL(...) \
    ___PRO_LHS_OP_DISPATCH_IMPL(BINARY, __VA_ARGS__) \
    ___PRO_RHS_OP_DISPATCH_IMPL(__VA_ARGS__)

#define ___PRO_DEF_LHS_ASSIGNMENT_OP_ACCESSOR(Q, SELF, ...) \
    template <class F, class C, class R, class Arg> \
    struct accessor<F, C, R(Arg) Q> { \
      ___PRO_GEN_SYMBOL_FOR_MEM_ACCESSOR(__VA_ARGS__) \
      decltype(auto) __VA_ARGS__ (Arg arg) Q { \
        proxy_invoke<C, R(Arg) Q>( \
            access_proxy<F>(SELF), std::forward<Arg>(arg)); \
        if constexpr (C::is_direct) { \
          return access_proxy<F>(SELF); \
        } else { \
          return *access_proxy<F>(SELF); \
        } \
      } \
    }
#ifdef NDEBUG
#define ___PRO_DEF_RHS_ASSIGNMENT_OP_ACCESSOR(Q, NE, SELF, FW_SELF, ...) \
    template <class F, class C, class R, class Arg> \
    struct accessor<F, C, R(Arg&) Q> { \
      friend Arg& operator __VA_ARGS__ (Arg& arg, SELF) NE { \
        proxy_invoke<C, R(Arg&) Q>(access_proxy<F>(FW_SELF), arg); \
        return arg; \
      } \
    }
#else
#define ___PRO_DEF_RHS_ASSIGNMENT_OP_ACCESSOR(Q, NE, SELF, FW_SELF, ...) \
    template <class F, class C, class R, class Arg> \
    struct accessor<F, C, R(Arg&) Q> { \
      accessor() noexcept { std::ignore = &accessor::_symbol_guard; } \
      friend Arg& operator __VA_ARGS__ (Arg& arg, SELF) NE { \
        proxy_invoke<C, R(Arg&) Q>(access_proxy<F>(FW_SELF), arg); \
        return arg; \
      } \
    \
     private: \
      static inline Arg& _symbol_guard(Arg& arg, SELF) NE \
          { return arg __VA_ARGS__ FW_SELF; } \
    }
#endif  // NDEBUG
#define ___PRO_ASSIGNMENT_OP_DISPATCH_IMPL(...) \
    template <> \
    struct operator_dispatch<#__VA_ARGS__, false> { \
      template <class T, class Arg> \
      decltype(auto) operator()(T&& self, Arg&& arg) \
          ___PRO_DIRECT_FUNC_IMPL(std::forward<T>(self) __VA_ARGS__ \
              std::forward<Arg>(arg)) \
      ___PRO_DEF_MEM_ACCESSOR_TEMPLATE(___PRO_DEF_LHS_ASSIGNMENT_OP_ACCESSOR, \
          operator __VA_ARGS__) \
    }; \
    template <> \
    struct operator_dispatch<#__VA_ARGS__, true> { \
      template <class T, class Arg> \
      decltype(auto) operator()(T&& self, Arg&& arg) \
          ___PRO_DIRECT_FUNC_IMPL(std::forward<Arg>(arg) __VA_ARGS__ \
              std::forward<T>(self)) \
      ___PRO_DEF_FREE_ACCESSOR_TEMPLATE(___PRO_DEF_RHS_ASSIGNMENT_OP_ACCESSOR, \
          __VA_ARGS__) \
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
  decltype(auto) operator()(T&& self, Args&&... args)
      ___PRO_DIRECT_FUNC_IMPL(
          std::forward<T>(self)(std::forward<Args>(args)...))
  ___PRO_DEF_MEM_ACCESSOR_TEMPLATE(___PRO_DEF_LHS_ANY_OP_ACCESSOR, operator())
};
template <>
struct operator_dispatch<"[]", false> {
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
  ___PRO_DEF_MEM_ACCESSOR_TEMPLATE(___PRO_DEF_LHS_ANY_OP_ACCESSOR, operator[])
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

#define ___PRO_DEF_CONVERSION_ACCESSOR(Q, SELF, ...) \
    template <class F, class C> \
    struct accessor<F, C, T() Q> { \
      ___PRO_GEN_SYMBOL_FOR_MEM_ACCESSOR(__VA_ARGS__) \
      explicit(Expl) __VA_ARGS__ () Q \
          { return proxy_invoke<C, T() Q>(access_proxy<F>(SELF)); } \
    }
template <class T, bool Expl = true>
struct conversion_dispatch {
  template <class U>
  T operator()(U&& value)
      noexcept(std::conditional_t<Expl, std::is_nothrow_constructible<T, U>,
          std::is_nothrow_convertible<U, T>>::value)
      requires(std::conditional_t<Expl, std::is_constructible<T, U>,
          std::is_convertible<U, T>>::value)
      { return static_cast<T>(std::forward<U>(value)); }
  ___PRO_DEF_MEM_ACCESSOR_TEMPLATE(___PRO_DEF_CONVERSION_ACCESSOR, operator T)
};
#undef ___PRO_DEF_CONVERSION_ACCESSOR

#define ___PRO_EXPAND_IMPL(__X) __X
#define ___PRO_EXPAND_MACRO_IMPL( \
    __MACRO, __1, __2, __3, __NAME, ...) \
    __MACRO##_##__NAME
#define ___PRO_EXPAND_MACRO(__MACRO, ...) \
    ___PRO_EXPAND_IMPL(___PRO_EXPAND_MACRO_IMPL( \
        __MACRO, __VA_ARGS__, 3, 2)(__VA_ARGS__))

#define ___PRO_DEF_MEM_ACCESSOR(__Q, __SELF, ...) \
    template <class __F, class __C, class __R, class... __Args> \
    struct accessor<__F, __C, __R(__Args...) __Q> { \
      ___PRO_GEN_SYMBOL_FOR_MEM_ACCESSOR(__VA_ARGS__) \
      __R __VA_ARGS__ (__Args... __args) __Q { \
        return ::pro::proxy_invoke<__C, __R(__Args...) __Q>( \
            ::pro::access_proxy<__F>(__SELF), \
            ::std::forward<__Args>(__args)...); \
      } \
    }
#define ___PRO_DEF_MEM_DISPATCH_IMPL(__NAME, __FUNC, __FNAME) \
    struct __NAME { \
      template <class __T, class... __Args> \
      decltype(auto) operator()(__T&& __self, __Args&&... __args) \
          ___PRO_DIRECT_FUNC_IMPL(::std::forward<__T>(__self) \
              .__FUNC(::std::forward<__Args>(__args)...)) \
      ___PRO_DEF_MEM_ACCESSOR_TEMPLATE(___PRO_DEF_MEM_ACCESSOR, __FNAME) \
    }
#define ___PRO_DEF_MEM_DISPATCH_2(__NAME, __FUNC) \
    ___PRO_DEF_MEM_DISPATCH_IMPL(__NAME, __FUNC, __FUNC)
#define ___PRO_DEF_MEM_DISPATCH_3(__NAME, __FUNC, __FNAME) \
    ___PRO_DEF_MEM_DISPATCH_IMPL(__NAME, __FUNC, __FNAME)
#define PRO_DEF_MEM_DISPATCH(__NAME, ...) \
    ___PRO_EXPAND_MACRO(___PRO_DEF_MEM_DISPATCH, __NAME, __VA_ARGS__)

#ifdef NDEBUG
#define ___PRO_DEF_FREE_ACCESSOR(__Q, __NE, __SELF, __FW_SELF, ...) \
    template <class __F, class __C, class __R, class... __Args> \
    struct accessor<__F, __C, __R(__Args...) __Q> { \
      friend __R __VA_ARGS__ (__SELF, __Args... __args) __NE { \
        return ::pro::proxy_invoke<__C, __R(__Args...) __Q>( \
            ::pro::access_proxy<__F>(__FW_SELF), \
            ::std::forward<__Args>(__args)...); \
      } \
    }
#else
#define ___PRO_DEF_FREE_ACCESSOR(__Q, __NE, __SELF, __FW_SELF, ...) \
    template <class __F, class __C, class __R, class... __Args> \
    struct accessor<__F, __C, __R(__Args...) __Q> { \
      accessor() noexcept { ::std::ignore = &accessor::_symbol_guard; } \
      friend __R __VA_ARGS__(__SELF, __Args... __args) __NE { \
        return ::pro::proxy_invoke<__C, __R(__Args...) __Q>( \
            ::pro::access_proxy<__F>(__FW_SELF), \
            ::std::forward<__Args>(__args)...); \
      } \
    \
     private: \
      static inline __R _symbol_guard(__SELF, __Args... __args) __NE { \
        return __VA_ARGS__(__FW_SELF, ::std::forward<__Args>(__args)...); \
      } \
    }
#endif  // NDEBUG
#define ___PRO_DEF_FREE_DISPATCH_IMPL(__NAME, __FUNC, __FNAME) \
    struct __NAME { \
      template <class __T, class... __Args> \
      decltype(auto) operator()(__T&& __self, __Args&&... __args) \
          ___PRO_DIRECT_FUNC_IMPL(__FUNC(::std::forward<__T>(__self), \
              ::std::forward<__Args>(__args)...)) \
      ___PRO_DEF_FREE_ACCESSOR_TEMPLATE(___PRO_DEF_FREE_ACCESSOR, __FNAME) \
    }
#define ___PRO_DEF_FREE_DISPATCH_2(__NAME, __FUNC) \
    ___PRO_DEF_FREE_DISPATCH_IMPL(__NAME, __FUNC, __FUNC)
#define ___PRO_DEF_FREE_DISPATCH_3(__NAME, __FUNC, __FNAME) \
    ___PRO_DEF_FREE_DISPATCH_IMPL(__NAME, __FUNC, __FNAME)
#define PRO_DEF_FREE_DISPATCH(__NAME, ...) \
    ___PRO_EXPAND_MACRO(___PRO_DEF_FREE_DISPATCH, __NAME, __VA_ARGS__)

#define ___PRO_DEF_FREE_AS_MEM_DISPATCH_IMPL(__NAME, __FUNC, __FNAME) \
    struct __NAME { \
      template <class __T, class... __Args> \
      decltype(auto) operator()(__T&& __self, __Args&&... __args) \
          ___PRO_DIRECT_FUNC_IMPL(__FUNC(::std::forward<__T>(__self), \
              ::std::forward<__Args>(__args)...)) \
      ___PRO_DEF_MEM_ACCESSOR_TEMPLATE(___PRO_DEF_MEM_ACCESSOR, __FNAME) \
    }
#define ___PRO_DEF_FREE_AS_MEM_DISPATCH_2(__NAME, __FUNC) \
    ___PRO_DEF_FREE_AS_MEM_DISPATCH_IMPL(__NAME, __FUNC, __FUNC)
#define ___PRO_DEF_FREE_AS_MEM_DISPATCH_3(__NAME, __FUNC, __FNAME) \
    ___PRO_DEF_FREE_AS_MEM_DISPATCH_IMPL(__NAME, __FUNC, __FNAME)
#define PRO_DEF_FREE_AS_MEM_DISPATCH(__NAME, ...) \
    ___PRO_EXPAND_MACRO(___PRO_DEF_FREE_AS_MEM_DISPATCH, __NAME, __VA_ARGS__)

#define PRO_DEF_WEAK_DISPATCH(__NAME, __D, __FUNC) \
    struct __NAME : __D { \
      using __D::operator(); \
      template <class... __Args> \
      decltype(auto) operator()(::std::nullptr_t, __Args&&... __args) \
          ___PRO_DIRECT_FUNC_IMPL(__FUNC(::std::forward<__Args>(__args)...)) \
    }

}  // namespace pro

#undef ___PRO_NO_UNIQUE_ADDRESS_ATTRIBUTE

#endif  // _MSFT_PROXY_
