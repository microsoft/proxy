// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef _MSFT_PROXY_
#define _MSFT_PROXY_

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

namespace details {

struct applicable_traits { static constexpr bool applicable = true; };
struct inapplicable_traits { static constexpr bool applicable = false; };

template <template <class, class> class R, class O, class... Is>
struct recursive_reduction : std::type_identity<O> {};
template <template <class, class> class R, class O, class I, class... Is>
struct recursive_reduction<R, O, I, Is...>
    : recursive_reduction<R, typename R<O, I>::type, Is...> {};
template <template <class, class> class R, class O, class... Is>
using recursive_reduction_t = typename recursive_reduction<R, O, Is...>::type;

template <template <class> class T, class... Is>
struct first_applicable {};
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

template <class T>
consteval bool is_tuple_like_well_formed() {
  if constexpr (requires { { std::tuple_size<T>::value } ->
      std::same_as<const std::size_t&>; }) {
    if constexpr (is_consteval([] { return std::tuple_size<T>::value; })) {
      return []<std::size_t... I>(std::index_sequence<I...>) {
            return (requires { typename std::tuple_element_t<I, T>; } && ...);
          }(std::make_index_sequence<std::tuple_size_v<T>>{});
    }
  }
  return false;
}

template <template <class...> class T, class TL, class Is> struct instantiated;
template <template <class...> class T, class TL, std::size_t... Is>
struct instantiated<T, TL, std::index_sequence<Is...>>
    { using type = T<std::tuple_element_t<Is, TL>...>; };
template <template <class...> class T, class TL>
using instantiated_t = typename instantiated<
    T, TL, std::make_index_sequence<std::tuple_size_v<TL>>>::type;

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
consteval bool requires_lifetime_meta(constraint_level level) {
  return level > constraint_level::none && level < constraint_level::trivial;
}

// As per std::to_address() wording in [pointer.conversion]
template <class P> struct ptr_traits : inapplicable_traits {};
template <class P>
    requires(requires(const P p) { std::pointer_traits<P>::to_address(p); } ||
        requires(const P p) { p.operator->(); })
struct ptr_traits<P> : applicable_traits {
  static auto to_address(const P& p) noexcept { return std::to_address(p); }
  using reference_type = typename ptr_traits<
      decltype(to_address(std::declval<const P&>()))>::reference_type;
};
template <class T>
struct ptr_traits<T*> : applicable_traits {
  static auto to_address(T* p) noexcept { return p; }
  using reference_type = T&;
};

template <class O> struct overload_traits : inapplicable_traits {};
template <class R, class... Args>
struct overload_traits<R(Args...)> : applicable_traits {
  using dispatcher_type = R (*)(const char*, Args...);
  struct resolver { R (*operator()(Args...))(Args...); };
  using forwarding_argument_types = std::tuple<Args&&...>;
  template <class D>
  struct meta_provider {
    template <class P>
    static R dispatcher(const char* erased, Args... args) {
      auto ptr = ptr_traits<P>::to_address(*reinterpret_cast<const P*>(erased));
      if constexpr (std::is_void_v<R>) {
        D{}(*ptr, std::forward<Args>(args)...);
      } else {
        return D{}(*ptr, std::forward<Args>(args)...);
      }
    }
  };

  template <class F, class T>
  static constexpr bool applicable_callable =
      std::is_invocable_r_v<R, F, T&, Args...>;
  template <class D, class P>
  static constexpr bool applicable_ptr =
      applicable_callable<D, typename ptr_traits<P>::reference_type>;
  static constexpr bool is_noexcept = false;
};
template <class R, class... Args>
struct overload_traits<R(Args...) noexcept> : applicable_traits {
  using dispatcher_type = R (*)(const char*, Args...) noexcept;
  struct resolver { R (*operator()(Args...))(Args...) noexcept; };
  using forwarding_argument_types = std::tuple<Args&&...>;
  template <class D>
  struct meta_provider {
    template <class P>
    static R dispatcher(const char* erased, Args... args) noexcept {
      auto ptr = ptr_traits<P>::to_address(*reinterpret_cast<const P*>(erased));
      if constexpr (std::is_void_v<R>) {
        D{}(*ptr, std::forward<Args>(args)...);
      } else {
        return D{}(*ptr, std::forward<Args>(args)...);
      }
    }
  };

  template <class F, class T>
  static constexpr bool applicable_callable =
      std::is_nothrow_invocable_r_v<R, F, T&, Args...>;
  template <class D, class P>
  static constexpr bool applicable_ptr =
      applicable_callable<D, typename ptr_traits<P>::reference_type>;
  static constexpr bool is_noexcept = true;
};

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
      : dispatcher(&MP::template dispatcher<P>) {}
  bool has_value() const noexcept { return dispatcher != nullptr; }
  void reset() noexcept { dispatcher = nullptr; }

  decltype(&MP::template dispatcher<void>) dispatcher;
};

template <class... Ms>
struct composite_meta : Ms... {
  static constexpr bool is_nullable =
      requires { typename first_applicable_t<nullable_traits, Ms...>; };

  constexpr composite_meta() noexcept requires(is_nullable) = default;
  template <class P>
  constexpr explicit composite_meta(std::in_place_type_t<P>) noexcept
      : Ms(std::in_place_type<P>)... {}

  bool has_value() const noexcept requires(is_nullable)
      { return first_applicable_t<nullable_traits, Ms...>::has_value(); }
  void reset() noexcept requires(is_nullable)
      { first_applicable_t<nullable_traits, Ms...>::reset(); }
};

template <class D>
struct dispatch_helper {
  template <class... Os> struct traits : inapplicable_traits {};
  template <class... Os>
      requires(sizeof...(Os) > 0u && (overload_traits<Os>::applicable && ...))
  struct traits<Os...> : applicable_traits {
   private:
    struct overload_resolver : overload_traits<Os>::resolver...
        { using overload_traits<Os>::resolver::operator()...; };

   public:
    using meta = composite_meta<dispatcher_meta<
        typename overload_traits<Os>::template meta_provider<D>>...>;
    template <class... Args>
    using matched_overload =
        std::remove_pointer_t<std::invoke_result_t<overload_resolver, Args...>>;

    template <class P>
    static constexpr bool applicable_ptr =
        (overload_traits<Os>::template applicable_ptr<D, P> && ...);
  };
};
template <class D> struct dispatch_traits : inapplicable_traits {};
template <class D>
    requires(requires { typename D::overload_types; } &&
        is_tuple_like_well_formed<typename D::overload_types>() &&
        std::is_trivially_default_constructible_v<D>)
struct dispatch_traits<D> : instantiated_t<
    dispatch_helper<D>::template traits, typename D::overload_types> {};

template <constraint_level C> struct copyability_meta_provider;
template <>
struct copyability_meta_provider<constraint_level::nontrivial> {
  template <class P>
  static void dispatcher(char* self, const char* rhs) {
    std::construct_at(reinterpret_cast<P*>(self),
        *reinterpret_cast<const P*>(rhs));
  }
};
template <>
struct copyability_meta_provider<constraint_level::nothrow> {
  template <class P>
  static void dispatcher(char* self, const char* rhs) noexcept {
    std::construct_at(reinterpret_cast<P*>(self),
        *reinterpret_cast<const P*>(rhs));
  }
};
template <constraint_level C> struct relocatability_meta_provider;
template <>
struct relocatability_meta_provider<constraint_level::nontrivial> {
  template <class P>
  static void dispatcher(char* self, char* rhs) {
    std::construct_at(reinterpret_cast<P*>(self),
        std::move(*reinterpret_cast<P*>(rhs)));
    std::destroy_at(reinterpret_cast<P*>(rhs));
  }
};
template <>
struct relocatability_meta_provider<constraint_level::nothrow> {
  template <class P>
  static void dispatcher(char* self, char* rhs) noexcept {
    std::construct_at(reinterpret_cast<P*>(self),
        std::move(*reinterpret_cast<P*>(rhs)));
    std::destroy_at(reinterpret_cast<P*>(rhs));
  }
};
template <constraint_level C> struct destructibility_meta_provider;
template <>
struct destructibility_meta_provider<constraint_level::nontrivial> {
  template <class P>
  static void dispatcher(char* self)
      { std::destroy_at(reinterpret_cast<P*>(self)); }
};
template <>
struct destructibility_meta_provider<constraint_level::nothrow> {
  template <class P>
  static void dispatcher(char* self) noexcept
      { std::destroy_at(reinterpret_cast<P*>(self)); }
};
template <template <constraint_level> class MP, constraint_level C>
using lifetime_meta = std::conditional_t<
    requires_lifetime_meta(C), dispatcher_meta<MP<C>>, void>;

template <class O, class I>
struct facade_meta_reduction : std::type_identity<O> {};
template <class... Ms, class I> requires(!std::is_void_v<I>)
struct facade_meta_reduction<composite_meta<Ms...>, I>
    : std::type_identity<composite_meta<Ms..., I>> {};

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
template <class... Ds>
struct default_dispatch_traits { using default_dispatch = void; };
template <class D>
struct default_dispatch_traits<D> { using default_dispatch = D; };
template <class F>
struct facade_helper {
  template <class... Ds> struct traits : inapplicable_traits {};
  template <class... Ds> requires(dispatch_traits<Ds>::applicable && ...)
  struct traits<Ds...> : applicable_traits, default_dispatch_traits<Ds...> {
    using copyability_meta = lifetime_meta<
        copyability_meta_provider, F::constraints.copyability>;
    using relocatability_meta = lifetime_meta<
        relocatability_meta_provider, F::constraints.relocatability>;
    using destructibility_meta = lifetime_meta<
        destructibility_meta_provider, F::constraints.destructibility>;
    using meta = recursive_reduction_t<facade_meta_reduction,
        composite_meta<>, copyability_meta, relocatability_meta,
        destructibility_meta, typename dispatch_traits<Ds>::meta...,
        typename F::reflection_type>;

    template <class D>
    static constexpr bool has_dispatch = (std::is_same_v<D, Ds> || ...);
    template <class P>
    static constexpr bool applicable_ptr =
        sizeof(P) <= F::constraints.max_size &&
        alignof(P) <= F::constraints.max_align &&
        has_copyability<P>(F::constraints.copyability) &&
        has_relocatability<P>(F::constraints.relocatability) &&
        has_destructibility<P>(F::constraints.destructibility) &&
        (dispatch_traits<Ds>::template applicable_ptr<P> && ...) &&
        is_reflection_type_well_formed<typename F::reflection_type, P>();
  };
};
template <class F> struct facade_traits : inapplicable_traits {};
template <class F>
    requires(
        requires {
          typename F::dispatch_types;
          { F::constraints } -> std::same_as<const proxiable_ptr_constraints&>;
          typename F::reflection_type;
        } && is_tuple_like_well_formed<typename F::dispatch_types>() &&
        is_facade_constraints_well_formed<F>())
struct facade_traits<F> : instantiated_t<
    facade_helper<F>::template traits, typename F::dispatch_types> {};

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
class proxy {
  using Traits = details::facade_traits<F>;
  static_assert(Traits::applicable);
  using DefaultDispatch = typename Traits::default_dispatch;
  template <class D, class... Args>
  using MatchedOverload =
      typename details::dispatch_traits<D>::template matched_overload<Args...>;

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
        memcpy(ptr_, rhs.ptr_, F::constraints.max_size);
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
  decltype(auto) reflect() const noexcept
      requires(!std::is_void_v<typename F::reflection_type>) {
    return *static_cast<const typename F::reflection_type*>(meta_.operator->());
  }
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
    initialize<P>(std::forward<Args>(args)...);
    return *reinterpret_cast<P*>(ptr_);
  }
  template <class P, class U, class... Args>
  P& emplace(std::initializer_list<U> il, Args&&... args)
      noexcept(HasNothrowPolyAssignment<P, std::initializer_list<U>&, Args...>)
      requires(HasPolyAssignment<P, std::initializer_list<U>&, Args...>) {
    reset();
    initialize<P>(il, std::forward<Args>(args)...);
    return *reinterpret_cast<P*>(ptr_);
  }
  template <class D = DefaultDispatch, class... Args>
  decltype(auto) invoke(Args&&... args) const
      noexcept(HasNothrowInvocation<D, Args...>)
      requires(Traits::template has_dispatch<D> &&
          requires { typename MatchedOverload<D, Args...>; }) {
    return meta_->template dispatcher_meta<typename details::overload_traits<
        MatchedOverload<D, Args...>>::template meta_provider<D>>
        ::dispatcher(ptr_, std::forward<Args>(args)...);
  }
  template <class... Args>
  decltype(auto) operator()(Args&&... args) const
      noexcept(HasNothrowInvocation<DefaultDispatch, Args...>)
      requires(requires { typename MatchedOverload<DefaultDispatch, Args...>; })
      { return invoke(std::forward<Args>(args)...); }

 private:
  template <class P, class... Args>
  void initialize(Args&&... args) {
    std::construct_at(reinterpret_cast<P*>(ptr_), std::forward<Args>(args)...);
    meta_ = details::meta_ptr<typename Traits::meta>{std::in_place_type<P>};
  }

  details::meta_ptr<typename Traits::meta> meta_;
  alignas(F::constraints.max_align) char ptr_[F::constraints.max_size];
};

constexpr proxiable_ptr_constraints relocatable_ptr_constraints{
  .max_size = sizeof(details::ptr_prototype),
  .max_align = alignof(details::ptr_prototype),
  .copyability = constraint_level::none,
  .relocatability = constraint_level::nothrow,
  .destructibility = constraint_level::nothrow,
};
constexpr proxiable_ptr_constraints copyable_ptr_constraints{
  .max_size = sizeof(details::ptr_prototype),
  .max_align = alignof(details::ptr_prototype),
  .copyability = constraint_level::nontrivial,
  .relocatability = constraint_level::nothrow,
  .destructibility = constraint_level::nothrow,
};
constexpr proxiable_ptr_constraints trivial_ptr_constraints{
  .max_size = sizeof(void*),
  .max_align = alignof(void*),
  .copyability = constraint_level::trivial,
  .relocatability = constraint_level::trivial,
  .destructibility = constraint_level::trivial,
};

namespace details {

template <class T>
class sbo_ptr {
 public:
  template <class... Args>
  sbo_ptr(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
      requires(std::is_constructible_v<T, Args...>)
      : value_(std::forward<Args>(args)...) {}
  sbo_ptr(const sbo_ptr&) noexcept(std::is_nothrow_copy_constructible_v<T>)
      = default;
  sbo_ptr(sbo_ptr&&) noexcept(std::is_nothrow_move_constructible_v<T>)
      = default;

  T* operator->() const noexcept { return &value_; }

 private:
  mutable T value_;
};

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
        : alloc(alloc), value(std::forward<Args>(args)...) {}

    Alloc alloc;
    T value;
  };

  storage* ptr_;
};

template <class F, class T, class Alloc, class... Args>
proxy<F> allocate_proxy_impl(const Alloc& alloc, Args&&... args) {
  if constexpr (proxiable<details::sbo_ptr<T>, F>) {
    return proxy<F>{std::in_place_type<details::sbo_ptr<T>>,
        std::forward<Args>(args)...};
  } else if constexpr (proxiable<details::allocated_ptr<T, Alloc>, F>) {
    return proxy<F>{std::in_place_type<details::allocated_ptr<T, Alloc>>,
        alloc, std::forward<Args>(args)...};
  } else {
    return proxy<F>{std::in_place_type<details::compact_ptr<T, Alloc>>,
        alloc, std::forward<Args>(args)...};
  }
}
template <class F, class T, class... Args>
proxy<F> make_proxy_impl(Args&&... args) {
  return allocate_proxy_impl<F, T>(
      std::allocator<T>{}, std::forward<Args>(args)...);
}

}  // namespace details

template <facade F, class T, class Alloc, class... Args>
proxy<F> allocate_proxy(const Alloc& alloc, Args&&... args) {
  return details::allocate_proxy_impl<F, T>(alloc, std::forward<Args>(args)...);
}
template <facade F, class T, class Alloc, class U, class... Args>
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

// The following types and macros aim to simplify definition of dispatch and
// facade types prior to C++26
namespace details {

template <class Args>
struct overload_matching_helper {
  template <class O> struct traits : inapplicable_traits {};
  template <class O>
      requires(std::is_same_v<
          typename overload_traits<O>::forwarding_argument_types, Args>)
  struct traits<O> : applicable_traits {};
};
template <class... Os>
struct dispatch_prototype_helper {
  template <class... Args>
  using overload = first_applicable_t<overload_matching_helper<
      std::tuple<Args&&...>>::template traits, Os...>;
  template <class... Args>
  static constexpr bool applicable = requires { typename overload<Args...>; };
  template <class... Args>
  static constexpr bool is_noexcept =
      overload_traits<overload<Args...>>::is_noexcept;
  template <class F, class T, class... Args>
  static constexpr bool applicable_callable =
      overload_traits<overload<Args...>>::template applicable_callable<F, T>;
};

template <class O, class I> struct flat_reduction : std::type_identity<O> {};
template <class O>
struct flat_reduction_helper {
  template <class... Is>
  using type = recursive_reduction<flat_reduction, O, Is...>;
};
template <class... Os, class I>
    requires(!is_tuple_like_well_formed<I>() && (!std::is_same_v<I, Os> && ...))
struct flat_reduction<std::tuple<Os...>, I>
    : std::type_identity<std::tuple<Os..., I>> {};
template <class O, class I> requires(is_tuple_like_well_formed<I>())
struct flat_reduction<O, I>
    : instantiated_t<flat_reduction_helper<O>::template type, I> {};
template <class O, class I>
struct overloads_reduction : std::type_identity<O> {};
template <class O, class I> requires(requires { typename I::overload_types; })
struct overloads_reduction<O, I>
    : flat_reduction<O, typename I::overload_types> {};

template <class... Os> requires(sizeof...(Os) > 0u)
struct dispatch_prototype { using overload_types = std::tuple<Os...>; };
template <class... Ds> requires(sizeof...(Ds) > 0u)
struct combined_dispatch_prototype : Ds... {
  using overload_types = recursive_reduction_t<
      overloads_reduction, std::tuple<>, Ds...>;
  using Ds::operator()...;
};
template <class Ds = std::tuple<>, proxiable_ptr_constraints C =
    relocatable_ptr_constraints, class R = void>
struct facade_prototype {
  using dispatch_types = typename flat_reduction<std::tuple<>, Ds>::type;
  static constexpr proxiable_ptr_constraints constraints = C;
  using reflection_type = R;
};

}  // namespace details

}  // namespace pro

#define ___PRO_DEF_DISPATCH_IMPL(NAME, EXPR, ...) \
    struct NAME : ::pro::details::dispatch_prototype<__VA_ARGS__> { \
     private: \
      using __helper = ::pro::details::dispatch_prototype_helper<__VA_ARGS__>; \
      struct __F { \
        template <class __T, class... __Args> \
        decltype(auto) operator()(__T& __self, __Args&&... __args) \
            noexcept(noexcept(EXPR)) requires(requires { EXPR; }) \
            { return EXPR; } \
      }; \
    \
     public: \
      template <class __T, class... __Args> \
      decltype(auto) operator()(__T& __self, __Args&&... __args) \
          noexcept(__helper::template is_noexcept<__Args...>) \
          requires(__helper::template applicable<__Args...> && \
              __helper::template applicable_callable<__F, __T, __Args...>) \
          { return __F{}(__self, std::forward<__Args>(__args)...); } \
    }
#define PRO_DEF_MEMBER_DISPATCH(NAME, ...) ___PRO_DEF_DISPATCH_IMPL( \
    NAME, __self.NAME(std::forward<__Args>(__args)...), __VA_ARGS__)
#define PRO_DEF_FREE_DISPATCH(NAME, FUNC, ...) ___PRO_DEF_DISPATCH_IMPL( \
    NAME, FUNC(__self, std::forward<__Args>(__args)...), __VA_ARGS__)
#define PRO_DEF_COMBINED_DISPATCH(NAME, ...) \
    struct NAME : ::pro::details::combined_dispatch_prototype<__VA_ARGS__> {}
#define PRO_MAKE_DISPATCH_PACK(...) std::tuple<__VA_ARGS__>
#define PRO_DEF_FACADE(NAME, ...) \
    struct NAME : ::pro::details::facade_prototype<__VA_ARGS__> {}

#endif  // _MSFT_PROXY_
