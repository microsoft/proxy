// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef _MSFT_PROXY_
#define _MSFT_PROXY_

#include <concepts>
#include <initializer_list>
#include <new>
#include <tuple>
#include <type_traits>
#include <utility>

namespace pro {

enum class constraint_level { none, nontrivial, nothrow, trivial };

template <class T, auto CPO = nullptr> struct dispatch;
template <class R, class... Args, auto CPO>
struct dispatch<R(Args...), CPO> {
  using return_type = R;
  using argument_types = std::tuple<Args...>;

  template <class T> requires(std::is_invocable_v<decltype(CPO)&, T, Args...>)
  constexpr decltype(auto) operator()(T&& value, Args&&... args) const
      { return CPO(std::forward<T>(value), std::forward<Args>(args)...); }
};

template <class... Ds>
struct facade {
  using dispatch_types = std::tuple<Ds...>;
  using reflection_type = void;
  static constexpr std::size_t maximum_size = sizeof(void*) * 2u;
  static constexpr std::size_t maximum_alignment = alignof(void*);
  static constexpr auto minimum_copyability = constraint_level::none;
  static constexpr auto minimum_relocatability = constraint_level::nothrow;
  static constexpr auto minimum_destructibility = constraint_level::nothrow;
  facade() = delete;
};

namespace details {

struct applicable_traits { static constexpr bool applicable = true; };
struct inapplicable_traits { static constexpr bool applicable = false; };

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

template <class P> struct pointer_traits : inapplicable_traits {};
template <class P> requires(requires(const P& ptr) { { *ptr }; })
struct pointer_traits<P> : applicable_traits
    { using value_type = decltype(*std::declval<const P&>()); };

template <class T, class... Us> struct contains_traits : inapplicable_traits {};
template <class T, class... Us>
struct contains_traits<T, T, Us...> : applicable_traits {};
template <class T, class U, class... Us>
struct contains_traits<T, U, Us...> : contains_traits<T, Us...> {};

template <class... U> struct default_traits { using type = void; };
template <class T> struct default_traits<T> { using type = T; };

template <class D, class Args>
struct dispatch_traits_impl : inapplicable_traits {};
template <class D, class... Args>
struct dispatch_traits_impl<D, std::tuple<Args...>> : applicable_traits {
  using dispatcher_type = typename D::return_type (*)(const char*, Args...);

  template <class T>
  static constexpr bool applicable_operand = requires(T operand, Args... args)
      { { D{}(std::forward<T>(operand), std::forward<Args>(args)...) }; };
  template <class P>
  static typename D::return_type dispatcher(const char* p, Args... args) {
    return D{}(**reinterpret_cast<const P*>(p), std::forward<Args>(args)...);
  }
};
template <class D> struct dispatch_traits : inapplicable_traits {};
template <class D> requires(requires {
      typename D::return_type;
      typename D::argument_types;
      { D{} };
    })
struct dispatch_traits<D>
    : dispatch_traits_impl<D, typename D::argument_types> {};

template <class D>
struct dispatch_meta {
  template <class P>
  constexpr explicit dispatch_meta(std::in_place_type_t<P>)
      : dispatcher(dispatch_traits<D>::template dispatcher<P>) {}

  typename dispatch_traits<D>::dispatcher_type dispatcher;
};
struct copy_meta {
  template <class P>
  constexpr explicit copy_meta(std::in_place_type_t<P>)
      : clone([](char* self, const char* rhs)
            { new(self) P(*reinterpret_cast<const P*>(rhs)); }) {}

  void (*clone)(char*, const char*);
};
struct relocation_meta {
  template <class P>
  constexpr explicit relocation_meta(std::in_place_type_t<P>)
      : relocate([](char* self, char* rhs) {
              new(self) P(std::move(*reinterpret_cast<P*>(rhs)));
              reinterpret_cast<P*>(rhs)->~P();
            }) {}

  void (*relocate)(char*, char*);
};
struct destruction_meta {
  template <class P>
  constexpr explicit destruction_meta(std::in_place_type_t<P>)
      : destroy([](char* self) { reinterpret_cast<P*>(self)->~P(); }) {}

  void (*destroy)(char*);
};
template <class... Ms>
struct facade_meta : Ms... {
  template <class P>
  constexpr explicit facade_meta(std::in_place_type_t<P>)
      : Ms(std::in_place_type<P>)... {}
};

template <constraint_level C, class M> struct conditional_meta_tag {};
template <class M, class Ms> struct facade_meta_traits_impl;
template <class M, class... Ms>
struct facade_meta_traits_impl<M, facade_meta<Ms...>>
    { using type = facade_meta<M, Ms...>; };
template <constraint_level C, class M, class... Ms>
    requires(C > constraint_level::none && C < constraint_level::trivial)
struct facade_meta_traits_impl<conditional_meta_tag<C, M>, facade_meta<Ms...>>
    { using type = facade_meta<M, Ms...>; };
template <constraint_level C, class M, class... Ms>
    requires(C < constraint_level::nontrivial || C > constraint_level::nothrow)
struct facade_meta_traits_impl<conditional_meta_tag<C, M>, facade_meta<Ms...>>
    { using type = facade_meta<Ms...>; };
template <class... Ms> struct facade_meta_traits;
template <class M, class... Ms>
struct facade_meta_traits<M, Ms...> : facade_meta_traits_impl<
    M, typename facade_meta_traits<Ms...>::type> {};
template <> struct facade_meta_traits<> { using type = facade_meta<>; };

template <class T, class U> struct flattening_traits_impl;
template <class T>
struct flattening_traits_impl<std::tuple<>, T> { using type = T; };
template <class T, class... Ts, class U>
struct flattening_traits_impl<std::tuple<T, Ts...>, U>
    : flattening_traits_impl<std::tuple<Ts...>, U> {};
template <class T, class... Ts, class... Us>
    requires(!contains_traits<T, Us...>::applicable)
struct flattening_traits_impl<std::tuple<T, Ts...>, std::tuple<Us...>>
    : flattening_traits_impl<std::tuple<Ts...>, std::tuple<Us..., T>> {};
template <class T> struct flattening_traits { using type = std::tuple<T>; };
template <>
struct flattening_traits<std::tuple<>> { using type = std::tuple<>; };
template <class T, class... Ts>
struct flattening_traits<std::tuple<T, Ts...>> : flattening_traits_impl<
    typename flattening_traits<T>::type,
    typename flattening_traits<std::tuple<Ts...>>::type> {};

template <class F, class Ds> struct basic_facade_traits_impl;
template <class F, class... Ds>
struct basic_facade_traits_impl<F, std::tuple<Ds...>> : applicable_traits {
  using meta_type = typename facade_meta_traits<
      conditional_meta_tag<F::minimum_copyability, copy_meta>,
      conditional_meta_tag<F::minimum_relocatability, relocation_meta>,
      conditional_meta_tag<F::minimum_destructibility, destruction_meta>,
      conditional_meta_tag<std::is_void_v<typename F::reflection_type> ?
          constraint_level::none : constraint_level::nothrow,
          typename F::reflection_type>>::type;
  using default_dispatch = typename default_traits<Ds...>::type;

  template <class D>
  static constexpr bool has_dispatch = contains_traits<D, Ds...>::applicable;
};
template <class F> struct basic_facade_traits : inapplicable_traits {};
template <class F> requires(requires {
      typename F::dispatch_types;
      typename F::reflection_type;
      typename std::integral_constant<std::size_t, F::maximum_size>;
      typename std::integral_constant<std::size_t, F::maximum_alignment>;
      typename std::integral_constant<constraint_level, F::minimum_copyability>;
      typename std::integral_constant<
          constraint_level, F::minimum_relocatability>;
      typename std::integral_constant<
          constraint_level, F::minimum_destructibility>;
    })
struct basic_facade_traits<F> : basic_facade_traits_impl<
    F, typename flattening_traits<typename F::dispatch_types>::type> {};

template <class F, class Ds>
struct facade_traits_impl : inapplicable_traits {};
template <class F, class... Ds> requires(dispatch_traits<Ds>::applicable && ...)
struct facade_traits_impl<F, std::tuple<Ds...>> : applicable_traits {
  using meta_type = facade_meta<
      typename basic_facade_traits<F>::meta_type, dispatch_meta<Ds>...>;

  template <class P>
  static constexpr bool applicable_pointer =
      sizeof(P) <= F::maximum_size && alignof(P) <= F::maximum_alignment &&
      has_copyability<P>(F::minimum_copyability) &&
      has_relocatability<P>(F::minimum_relocatability) &&
      has_destructibility<P>(F::minimum_destructibility) &&
      (dispatch_traits<Ds>::template applicable_operand<
          typename pointer_traits<P>::value_type> && ...) &&
      (std::is_void_v<typename F::reflection_type> || std::is_constructible_v<
          typename F::reflection_type, std::in_place_type_t<P>>);
  template <class P> static constexpr meta_type meta{std::in_place_type<P>};
};
template <class F> struct facade_traits : facade_traits_impl<
    F, typename flattening_traits<typename F::dispatch_types>::type> {};

template <class T, class U> struct dependent_traits { using type = T; };
template <class T, class U>
using dependent_t = typename dependent_traits<T, U>::type;

}  // namespace details

template <class P, class F>
concept proxiable = details::pointer_traits<P>::applicable &&
    details::basic_facade_traits<F>::applicable &&
    details::facade_traits<F>::applicable &&
    details::facade_traits<F>::template applicable_pointer<P>;

template <class F> requires(details::basic_facade_traits<F>::applicable)
class proxy {
  using BasicTraits = details::basic_facade_traits<F>;
  using Traits = details::facade_traits<F>;

  template <class P, class... Args>
  static constexpr bool HasNothrowPolyConstructor = std::conditional_t<
      proxiable<P, F>, std::is_nothrow_constructible<P, Args...>,
          std::false_type>::value;
  template <class P, class... Args>
  static constexpr bool HasPolyConstructor = std::conditional_t<
      proxiable<P, F>, std::is_constructible<P, Args...>,
          std::false_type>::value;
  static constexpr bool HasTrivialCopyConstructor =
      F::minimum_copyability == constraint_level::trivial;
  static constexpr bool HasNothrowCopyConstructor =
      F::minimum_copyability >= constraint_level::nothrow;
  static constexpr bool HasCopyConstructor =
      F::minimum_copyability >= constraint_level::nontrivial;
  static constexpr bool HasNothrowMoveConstructor =
      F::minimum_relocatability >= constraint_level::nothrow;
  static constexpr bool HasMoveConstructor =
      F::minimum_relocatability >= constraint_level::nontrivial;
  static constexpr bool HasTrivialDestructor =
      F::minimum_destructibility == constraint_level::trivial;
  static constexpr bool HasNothrowDestructor =
      F::minimum_destructibility >= constraint_level::nothrow;
  static constexpr bool HasDestructor =
      F::minimum_destructibility >= constraint_level::nontrivial;
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
  proxy() noexcept { meta_ = nullptr; }
  proxy(std::nullptr_t) noexcept : proxy() {}
  proxy(const proxy& rhs) noexcept(HasNothrowCopyConstructor)
      requires(!HasTrivialCopyConstructor && HasCopyConstructor) {
    if (rhs.meta_ != nullptr) {
      rhs.meta_->clone(ptr_, rhs.ptr_);
      meta_ = rhs.meta_;
    } else {
      meta_ = nullptr;
    }
  }
  proxy(const proxy&) noexcept requires(HasTrivialCopyConstructor) = default;
  proxy(const proxy&) requires(!HasCopyConstructor) = delete;
  proxy(proxy&& rhs) noexcept(HasNothrowMoveConstructor)
      requires(HasMoveConstructor) {
    if (rhs.meta_ != nullptr) {
      if constexpr (F::minimum_relocatability == constraint_level::trivial) {
        memcpy(ptr_, rhs.ptr_, F::maximum_size);
      } else {
        rhs.meta_->relocate(ptr_, rhs.ptr_);
      }
      meta_ = rhs.meta_;
      rhs.meta_ = nullptr;
    } else {
      meta_ = nullptr;
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
    this->~proxy();
    new(this) proxy();
    return *this;
  }
  proxy& operator=(const proxy& rhs)
      requires(!HasNothrowCopyAssignment && HasCopyAssignment)
      { return *this = proxy{rhs}; }
  proxy& operator=(const proxy& rhs) noexcept
      requires(!HasTrivialCopyAssignment && HasNothrowCopyAssignment) {
    if (this != &rhs) {
      this->~proxy();
      new(this) proxy(rhs);
    }
    return *this;
  }
  proxy& operator=(const proxy&) noexcept requires(HasTrivialCopyAssignment) =
      default;
  proxy& operator=(const proxy&) requires(!HasCopyAssignment) = delete;
  proxy& operator=(proxy&& rhs) noexcept(HasNothrowMoveAssignment)
    requires(HasMoveAssignment) {
    if constexpr (HasNothrowMoveAssignment) {
      this->~proxy();
    } else {
      reset();  // For weak exception safety
    }
    new(this) proxy(std::move(rhs));
    return *this;
  }
  proxy& operator=(proxy&&) requires(!HasMoveAssignment) = delete;
  template <class P>
  proxy& operator=(P&& ptr) noexcept
      requires(HasNothrowPolyAssignment<std::decay_t<P>, P>) {
    this->~proxy();
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
    if (meta_ != nullptr) {
      meta_->destroy(ptr_);
    }
  }
  ~proxy() requires(HasTrivialDestructor) = default;
  ~proxy() requires(!HasDestructor) = delete;

  bool has_value() const noexcept { return meta_ != nullptr; }
  decltype(auto) reflect() const noexcept
      requires(!std::is_void_v<typename F::reflection_type>)
      { return static_cast<const typename F::reflection_type&>(*meta_); }
  void reset() noexcept(HasNothrowDestructor) requires(HasDestructor)
      { this->~proxy(); meta_ = nullptr; }
  void swap(proxy& rhs) noexcept(HasNothrowMoveConstructor)
      requires(HasMoveConstructor) {
    if constexpr (F::minimum_relocatability == constraint_level::trivial) {
      std::swap(meta_, rhs.meta_);
      std::swap(ptr_, rhs.ptr);
    } else {
      if (meta_ != nullptr) {
        if (rhs.meta_ != nullptr) {
          proxy temp = std::move(*this);
          new(this) proxy(std::move(rhs));
          new(&rhs) proxy(std::move(temp));
        } else {
          new(&rhs) proxy(std::move(*this));
        }
      } else if (rhs.meta_ != nullptr) {
        new(this) proxy(std::move(rhs));
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
  template <class D = typename BasicTraits::default_dispatch, class... Args>
  decltype(auto) invoke(Args&&... args) const
      requires(details::dependent_t<Traits, D>::applicable &&
          BasicTraits::template has_dispatch<D> &&
          std::is_convertible_v<std::tuple<Args...>,
              typename D::argument_types>) {
    return static_cast<const typename Traits::meta_type*>(meta_)
        ->template dispatch_meta<D>
        ::dispatcher(ptr_, std::forward<Args>(args)...);
  }

 private:
  template <class P, class... Args>
  void initialize(Args&&... args) {
    new(ptr_) P(std::forward<Args>(args)...);
    meta_ = &Traits::template meta<P>;
  }

  const typename BasicTraits::meta_type* meta_;
  alignas(F::maximum_alignment) char ptr_[F::maximum_size];
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

  T& operator*() const { return value_; }

 private:
  mutable T value_;
};

template <class T>
class deep_ptr {
 public:
  template <class... Args>
  deep_ptr(Args&&... args) requires(std::is_constructible_v<T, Args...>)
      : ptr_(new T(std::forward<Args>(args)...)) {}
  deep_ptr(const deep_ptr& rhs) requires(std::is_copy_constructible_v<T>)
      : ptr_(rhs.ptr_ == nullptr ? nullptr : new T(*rhs)) {}
  deep_ptr(deep_ptr&& rhs) noexcept : ptr_(rhs.ptr_) { rhs.ptr_ = nullptr; }
  ~deep_ptr() noexcept { delete ptr_; }

  T& operator*() const { return *ptr_; }

 private:
  T* ptr_;
};

template <class F, class T, class... Args>
proxy<F> make_proxy_impl(Args&&... args) {
  return proxy<F>{std::in_place_type<
      std::conditional_t<proxiable<sbo_ptr<T>, F>, sbo_ptr<T>, deep_ptr<T>>>,
      std::forward<Args>(args)...};
}

}  // namespace details

template <class F, class T, class... Args>
proxy<F> make_proxy(Args&&... args)
    { return details::make_proxy_impl<F, T>(std::forward<Args>(args)...); }
template <class F, class T, class U, class... Args>
proxy<F> make_proxy(std::initializer_list<U> il, Args&&... args)
    { return details::make_proxy_impl<F, T>(il, std::forward<Args>(args)...); }
template <class F, class T>
proxy<F> make_proxy(T&& value) {
  return details::make_proxy_impl<F, std::decay_t<T>>(std::forward<T>(value));
}

}  // namespace pro

#endif  // _MSFT_PROXY_
