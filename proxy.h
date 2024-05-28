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
concept invocable_dispatch_ptr = invocable_dispatch<
    D, NE, R, typename ptr_traits<P>::target_type&, Args...> ||
    invocable_dispatch<D, NE, R, const P&, Args...> ||
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
R invocation_dispatcher_ref(const std::byte* self, Args... args)
    noexcept(invocable_dispatch<
        D, true, R, typename ptr_traits<P>::target_type&, Args...>) {
  return invoke_dispatch<D, R>(ptr_traits<P>::dereference(*std::launder(
      reinterpret_cast<const P*>(self))), std::forward<Args>(args)...);
}
template <class P, class D, class R, class... Args>
R invocation_dispatcher_ptr(const std::byte* self, Args... args)
    noexcept(invocable_dispatch<D, true, R, const P&, Args...>) {
  return invoke_dispatch<D, R>(*std::launder(reinterpret_cast<const P*>(self)),
      std::forward<Args>(args)...);
}
template <class D, class R, class... Args>
R invocation_dispatcher_default(const std::byte*, Args... args)
    noexcept(invocable_dispatch<D, true, R, std::nullptr_t, Args...>)
    { return invoke_dispatch<D, R>(nullptr, std::forward<Args>(args)...); }
template <class P>
void copying_dispatcher(std::byte* self, const std::byte* rhs)
    noexcept(has_copyability<P>(constraint_level::nothrow)) {
  std::construct_at(reinterpret_cast<P*>(self),
      *std::launder(reinterpret_cast<const P*>(rhs)));
}
template <std::size_t Len, std::size_t Align>
void copying_default_dispatcher(std::byte* self, const std::byte* rhs)
    noexcept {
  std::uninitialized_copy_n(
      std::assume_aligned<Align>(rhs), Len, std::assume_aligned<Align>(self));
}
template <class P>
void relocation_dispatcher(std::byte* self, const std::byte* rhs)
    noexcept(has_relocatability<P>(constraint_level::nothrow)) {
  P* other = std::launder(reinterpret_cast<P*>(const_cast<std::byte*>(rhs)));
  std::construct_at(reinterpret_cast<P*>(self), std::move(*other));
  std::destroy_at(other);
}
template <class P>
void destruction_dispatcher(std::byte* self)
    noexcept(has_destructibility<P>(constraint_level::nothrow))
    { std::destroy_at(std::launder(reinterpret_cast<P*>(self))); }
inline void destruction_default_dispatcher(std::byte*) noexcept {}

template <bool NE, class R, class... Args>
struct overload_traits_impl : applicable_traits {
  template <class D>
  struct meta_provider {
    template <class P>
    static constexpr func_ptr_t<NE, R, const std::byte*, Args...> get() {
      if constexpr (invocable_dispatch<
          D, NE, R, typename ptr_traits<P>::target_type&, Args...>) {
        return &invocation_dispatcher_ref<P, D, R, Args...>;
      } else if constexpr (invocable_dispatch<D, NE, R, const P&, Args...>) {
        return &invocation_dispatcher_ptr<P, D, R, Args...>;
      } else {
        return &invocation_dispatcher_default<D, R, Args...>;
      }
    }
  };
  struct resolver { func_ptr_t<NE, R, Args...> operator()(Args...); };
  template <class D, class P>
  static constexpr bool applicable_ptr =
      invocable_dispatch_ptr<D, P, NE, R, Args...>;
  static constexpr bool is_noexcept = NE;
};
template <class O> struct overload_traits : inapplicable_traits {};
template <class R, class... Args>
struct overload_traits<R(Args...)> : overload_traits_impl<false, R, Args...> {};
template <class R, class... Args>
struct overload_traits<R(Args...) noexcept>
    : overload_traits_impl<true, R, Args...> {};

template <class T>
struct nullable_traits : conditional_traits<requires(const T& cv, T& v) {
      { T{} } noexcept;
      { cv.has_value() } noexcept -> std::same_as<bool>;
      { v.reset() } noexcept;
    }> {};

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
template <class... Ms0, class... Ms1>
struct meta_reduction<composite_meta_impl<Ms0...>, composite_meta_impl<Ms1...>>
    : std::type_identity<composite_meta_impl<Ms0..., Ms1...>> {};
template <class O, class I>
using meta_reduction_t = typename meta_reduction<O, I>::type;
template <class... Ms>
using composite_meta =
    recursive_reduction_t<meta_reduction_t, composite_meta_impl<>, Ms...>;

template <class C, class... Os>
struct conv_traits_impl : inapplicable_traits {};
template <class C, class... Os>
    requires(sizeof...(Os) > 0u && (overload_traits<Os>::applicable && ...))
struct conv_traits_impl<C, Os...> : applicable_traits {
 private:
  struct overload_resolver : overload_traits<Os>::resolver...
      { using overload_traits<Os>::resolver::operator()...; };

 public:
  using dispatch_type = typename C::dispatch_type;
  using meta = composite_meta<dispatcher_meta<
      typename overload_traits<Os>::template meta_provider<dispatch_type>>...>;
  template <class... Args>
  using matched_overload =
      std::remove_pointer_t<std::invoke_result_t<overload_resolver, Args...>>;

  template <class P>
  static constexpr bool applicable_ptr =
      (overload_traits<Os>::template applicable_ptr<dispatch_type, P> && ...);
};
template <class C> struct conv_traits : inapplicable_traits {};
template <class C>
    requires(
        requires {
          typename C::dispatch_type;
          typename C::overload_types;
        } &&
        std::is_nothrow_default_constructible_v<typename C::dispatch_type> &&
        is_tuple_like_well_formed<typename C::overload_types>())
struct conv_traits<C> : instantiated_t<
    conv_traits_impl, typename C::overload_types, C> {};

template <bool NE>
struct copyability_meta_provider {
  template <class P>
  static constexpr func_ptr_t<NE, void, std::byte*, const std::byte*> get() {
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
  static constexpr func_ptr_t<NE, void, std::byte*, const std::byte*> get() {
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
  static constexpr func_ptr_t<NE, void, std::byte*> get() {
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

template <class... As> struct composite_accessor_impl : As... {};
template <class T, class O, class I>
struct accessor_reduction : std::type_identity<O> {};
template <class T, class... As, class I>
    requires(requires { typename I::template accessor<T>; } &&
        std::is_nothrow_default_constructible_v<
            typename I::template accessor<T>>)
struct accessor_reduction<T, composite_accessor_impl<As...>, I>
    : std::type_identity<composite_accessor_impl<
          As..., typename I::template accessor<T>>> {};
template <class T, class... As0, class... As1>
struct accessor_reduction<
    T, composite_accessor_impl<As0...>, composite_accessor_impl<As1...>>
    : std::type_identity<composite_accessor_impl<As0..., As1...>> {};
template <class T>
struct accessor_helper {
  template <class O, class I>
  using reduction_t = typename accessor_reduction<T, O, I>::type;
};
template <class F, class... As>
using composite_accessor = recursive_reduction_t<accessor_helper<proxy<F>>
    ::template reduction_t, composite_accessor_impl<>, As...>;

template <class F>
consteval bool is_facade_constraints_well_formed() {
  if constexpr (is_consteval([] { return F::constraints; })) {
    return std::has_single_bit(F::constraints.max_align) &&
        F::constraints.max_size % F::constraints.max_align == 0u;
  }
  return false;
}
template <class R, class P>
consteval bool is_reflection_type_well_formed() {
  if constexpr (std::is_void_v<R>) {
    return true;
  } else if constexpr (std::is_constructible_v<R, std::in_place_type_t<P>>) {
    return is_consteval([] { return R{std::in_place_type<P>}; });
  }
  return false;
}
template <class D>
struct dispatch_match_helper {
  template <class C>
  using traits =
      conditional_traits<std::is_same_v<typename C::dispatch_type, D>>;
};
template <class F, class... Cs>
struct facade_conv_traits_impl : inapplicable_traits {};
template <class F, class... Cs> requires(conv_traits<Cs>::applicable && ...)
struct facade_conv_traits_impl<F, Cs...> : applicable_traits {
  using conv_meta = composite_meta<typename conv_traits<Cs>::meta...>;
  using conv_accessor = composite_accessor<
      F, typename conv_traits<Cs>::dispatch_type...>;
  template <class D, class... Args>
  using matched_overload = typename conv_traits<
      first_applicable_t<dispatch_match_helper<D>::template traits, Cs...>>
      ::template matched_overload<Args...>;

  template <class P>
  static constexpr bool conv_applicable_ptr =
      (conv_traits<Cs>::template applicable_ptr<P> && ...);
};
template <class F, class... Rs>
struct facade_refl_traits_impl {
  using refl_meta = composite_meta<Rs...>;
  using refl_accessor = composite_accessor<F, Rs...>;

  template <class R>
  static constexpr bool has_refl = (std::is_same_v<R, Rs> || ...);
  template <class P>
  static constexpr bool refl_applicable_ptr =
      (is_reflection_type_well_formed<Rs, P>() && ...);
};
template <class F> struct facade_traits : inapplicable_traits {};
template <class F>
    requires(
        requires {
          typename F::convention_types;
          typename F::reflection_types;
          { F::constraints } -> std::same_as<const proxiable_ptr_constraints&>;
        } && is_tuple_like_well_formed<typename F::convention_types>() &&
        is_tuple_like_well_formed<typename F::reflection_types>() &&
        is_facade_constraints_well_formed<F>() &&
        instantiated_t<facade_conv_traits_impl,
            typename F::convention_types, F>::applicable)
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
  using base = composite_accessor<F, typename facade_traits::conv_accessor,
      typename facade_traits::refl_accessor>;

  template <class P>
  static constexpr bool applicable_ptr =
      sizeof(P) <= F::constraints.max_size &&
      alignof(P) <= F::constraints.max_align &&
      has_copyability<P>(F::constraints.copyability) &&
      has_relocatability<P>(F::constraints.relocatability) &&
      has_destructibility<P>(F::constraints.destructibility) &&
      facade_traits::template conv_applicable_ptr<P> &&
      facade_traits::template refl_applicable_ptr<P>;
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

}  // namespace details

template <class F>
concept facade = details::facade_traits<F>::applicable;

template <class P, class F>
concept proxiable = facade<F> && details::ptr_traits<P>::applicable &&
    details::facade_traits<F>::template applicable_ptr<P>;

template <class F>
class proxy : public details::facade_traits<F>::base {
  using Traits = details::facade_traits<F>;
  static_assert(Traits::applicable);
  template <class D, class... Args>
  using MatchedOverload =
      typename Traits::template matched_overload<D, Args...>;

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
  template <class D, class... Args>
  static constexpr bool HasNothrowInvocation =
      details::overload_traits<MatchedOverload<D, Args...>>::is_noexcept;

 public:
  proxy() noexcept = default;
  proxy(std::nullptr_t) noexcept : proxy() {}
  proxy(const proxy& rhs) noexcept(HasNothrowCopyConstructor)
      requires(!HasTrivialCopyConstructor && HasCopyConstructor) {
    if (rhs.meta_.has_value()) {
      rhs.meta_->Traits::copyability_meta::dispatcher(ptr_, rhs.ptr_);
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
        rhs.meta_->Traits::relocatability_meta::dispatcher(ptr_, rhs.ptr_);
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
      meta_->Traits::destructibility_meta::dispatcher(ptr_);
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
  template <class D, class... Args>
  decltype(auto) invoke(Args&&... args) const
      noexcept(HasNothrowInvocation<D, Args...>)
      requires(requires { typename MatchedOverload<D, Args...>; }) {
    return meta_->template dispatcher_meta<typename details::overload_traits<
        MatchedOverload<D, Args...>>::template meta_provider<D>>
        ::dispatcher(ptr_, std::forward<Args>(args)...);
  }
  template <class R>
  const R& reflect() const noexcept requires(Traits::template has_refl<R>)
      { return *static_cast<const R*>(meta_.operator->()); }

 private:
  template <class P, class... Args>
  P& initialize(Args&&... args) {
    std::construct_at(reinterpret_cast<P*>(ptr_), std::forward<Args>(args)...);
    meta_ = details::meta_ptr<typename Traits::meta>{std::in_place_type<P>};
    return *std::launder(reinterpret_cast<P*>(ptr_));
  }

  details::meta_ptr<typename Traits::meta> meta_;
  alignas(F::constraints.max_align) std::byte ptr_[F::constraints.max_size];
};

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
#if __has_cpp_attribute(msvc::no_unique_address)
  [[msvc::no_unique_address]]
#elif __has_cpp_attribute(no_unique_address)
  [[__no_unique_address__]]
#endif
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

template <class... Args> void invalid_call(Args&&...) = delete;
template <class T, class...> struct lazy_eval_traits : std::type_identity<T> {};

constexpr std::size_t invalid_size = static_cast<std::size_t>(-1);
constexpr constraint_level invalid_cl = static_cast<constraint_level>(-1);
consteval auto normalize(proxiable_ptr_constraints value) {
  if (value.max_size == invalid_size) {
    value.max_size = sizeof(ptr_prototype);
  }
  if (value.max_align == invalid_size) {
    value.max_align = alignof(ptr_prototype);
  }
  if (value.copyability == invalid_cl) {
    value.copyability = constraint_level::none;
  }
  if (value.relocatability == invalid_cl) {
    value.relocatability = constraint_level::nothrow;
  }
  if (value.destructibility == invalid_cl) {
    value.destructibility = constraint_level::nothrow;
  }
  return value;
}
consteval auto make_restricted_layout(proxiable_ptr_constraints value,
    std::size_t max_size, std::size_t max_align) {
  if (value.max_size == invalid_size || value.max_size > max_size) {
    value.max_size = max_size;
  }
  if (value.max_align == invalid_size || value.max_align > max_align) {
    value.max_align = max_align;
  }
  return value;
}
consteval auto make_copyable(proxiable_ptr_constraints value,
    constraint_level cl) {
  if (value.copyability == invalid_cl || value.copyability < cl) {
    value.copyability = cl;
  }
  return value;
}
consteval auto make_relocatable(proxiable_ptr_constraints value,
    constraint_level cl) {
  if (value.relocatability == invalid_cl || value.relocatability < cl) {
    value.relocatability = cl;
  }
  return value;
}
consteval auto make_destructible(proxiable_ptr_constraints value,
    constraint_level cl) {
  if (value.destructibility == invalid_cl || value.destructibility < cl) {
    value.destructibility = cl;
  }
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
      requires(std::is_nothrow_default_constructible_v<D> &&
          (overload_traits<Os>::applicable && ...))
  using add_convention = facade_builder_impl<add_conv_t<
      Cs, conv_impl<D, std::tuple<Os...>>>, Rs, C>;
  template <class R>
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

template <class U, class... T>
using lazy_eval_t = typename details::lazy_eval_traits<U, T...>::type;
template <class... T>
auto lazy_eval(auto& value) noexcept -> lazy_eval_t<decltype(value), T...>
    { return value; }

using facade_builder = details::facade_builder_impl<std::tuple<>, std::tuple<>,
    proxiable_ptr_constraints{
        .max_size = details::invalid_size,
        .max_align = details::invalid_size,
        .copyability = details::invalid_cl,
        .relocatability = details::invalid_cl,
        .destructibility = details::invalid_cl}>;

}  // namespace pro

#define ___PRO_DIRECT_FUNC_IMPL(__EXPR) \
    noexcept(noexcept(__EXPR)) requires(requires { __EXPR; }) { return __EXPR; }
#define PRO_DEF_MEM_DISPATCH_WITH_DEFAULT(__NAME, __FUNC, __DEFFUNC) \
    struct __NAME { \
      using __name = __NAME; \
      template <class __T, class... __Args> \
      decltype(auto) operator()(__T& __self, __Args&&... __args) \
          ___PRO_DIRECT_FUNC_IMPL( \
              __self.__FUNC(::std::forward<__Args>(__args)...)) \
      template <class... __Args> \
      decltype(auto) operator()(std::nullptr_t, __Args&&... __args) \
          ___PRO_DIRECT_FUNC_IMPL( \
              __DEFFUNC(::std::forward<__Args>(__args)...)) \
      template <class __P> \
      struct accessor { \
        template <class... __Args> \
        decltype(auto) __FUNC(__Args&&... __args) const \
            ___PRO_DIRECT_FUNC_IMPL((static_cast< \
                ::pro::lazy_eval_t<const __P&, __Args...>>(*this) \
                .template invoke<__name>(::std::forward<__Args>(__args)...))) \
      }; \
    }
#define PRO_DEF_FREE_DISPATCH_WITH_DEFAULT(__NAME, __FNAME, __FUNC, __DEFFUNC) \
    struct __NAME { \
      using __name = __NAME; \
      template <class __T, class... __Args> \
      decltype(auto) operator()(__T& __self, __Args&&... __args) \
          ___PRO_DIRECT_FUNC_IMPL( \
              __FUNC(__self, ::std::forward<__Args>(__args)...)) \
      template <class... __Args> \
      decltype(auto) operator()(std::nullptr_t, __Args&&... __args) \
          ___PRO_DIRECT_FUNC_IMPL( \
              __DEFFUNC(::std::forward<__Args>(__args)...)) \
      template <class __P> \
      struct accessor { \
        template <class... __Args> \
        friend decltype(auto) __FNAME(const __P& __self, __Args&&... __args) \
            ___PRO_DIRECT_FUNC_IMPL((::pro::lazy_eval<__Args...>(__self) \
                .template invoke<__name>(::std::forward<__Args>(__args)...))) \
      }; \
    }
#define PRO_DEF_MEM_DISPATCH(__NAME, __FUNC) \
    PRO_DEF_MEM_DISPATCH_WITH_DEFAULT( \
        __NAME, __FUNC, ::pro::details::invalid_call)
#define PRO_DEF_FREE_DISPATCH(__NAME, __FNAME, __FUNC) \
    PRO_DEF_FREE_DISPATCH_WITH_DEFAULT( \
        __NAME, __FNAME, __FUNC, ::pro::details::invalid_call)

#endif  // _MSFT_PROXY_
