// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#ifndef MSFT_PROXY_V4_PROXY_MACROS_H_
#define MSFT_PROXY_V4_PROXY_MACROS_H_

#if __cpp_static_call_operator >= 202207L
#define PRO4D_STATIC_CALL(ret, ...) static ret operator()(__VA_ARGS__)
#else
#define PRO4D_STATIC_CALL(ret, ...) ret operator()(__VA_ARGS__) const
#endif // __cpp_static_call_operator >= 202207L

#if __cpp_exceptions >= 199711L
#define PRO4D_THROW(...) throw __VA_ARGS__
#else
#define PRO4D_THROW(...) std::abort()
#endif // __cpp_exceptions >= 199711L

#ifdef _MSC_VER
#define PRO4D_ENFORCE_EBO __declspec(empty_bases)
#else
#define PRO4D_ENFORCE_EBO
#endif // _MSC_VER

#ifdef NDEBUG
#define PRO4D_DEBUG(...)
#else
#define PRO4D_DEBUG(...) __VA_ARGS__
#endif // NDEBUG

#define __msft_lib_proxy4 202510L

#define PRO4D_DIRECT_FUNC_IMPL(...)                                            \
  noexcept(noexcept(__VA_ARGS__))                                              \
    requires(requires { __VA_ARGS__; })                                        \
  {                                                                            \
    return __VA_ARGS__;                                                        \
  }

#define PRO4D_DEF_OVERLOAD_SPECIALIZATIONS(macro, ...)                         \
  macro(, &, , __VA_ARGS__);                                                   \
  macro(, &, noexcept, __VA_ARGS__);                                           \
  macro(&, &, , __VA_ARGS__);                                                  \
  macro(&, &, noexcept, __VA_ARGS__);                                          \
  macro(&&, &&, , __VA_ARGS__);                                                \
  macro(&&, &&, noexcept, __VA_ARGS__);                                        \
  macro(const, const&, , __VA_ARGS__);                                         \
  macro(const, const&, noexcept, __VA_ARGS__);                                 \
  macro(const&, const&, , __VA_ARGS__);                                        \
  macro(const&, const&, noexcept, __VA_ARGS__);                                \
  macro(const&&, const&&, , __VA_ARGS__);                                      \
  macro(const&&, const&&, noexcept, __VA_ARGS__);

#define PRO4D_DEF_AGGREGATE_MEM_ACCESSOR_BODY(...)                             \
  using accessor<ProP, ProD, ProOs>::__VA_ARGS__...;
#define PRO4D_DEF_AGGREGATE_FREE_ACCESSOR_BODY(...)
#define PRO4D_DEF_ACCESSOR_TEMPLATE(type, macro, ...)                          \
  template <class ProP, class ProD, class... ProOs>                            \
  struct PRO4D_ENFORCE_EBO accessor {                                          \
    accessor() = delete;                                                       \
  };                                                                           \
  template <class ProP, class ProD, class... ProOs>                            \
    requires(sizeof...(ProOs) > 1u &&                                          \
             (::std::is_constructible_v<accessor<ProP, ProD, ProOs>> && ...))  \
  struct accessor<ProP, ProD, ProOs...> : accessor<ProP, ProD, ProOs>... {     \
    PRO4D_DEF_AGGREGATE_##type##_ACCESSOR_BODY(__VA_ARGS__)                    \
  };                                                                           \
  PRO4D_DEF_OVERLOAD_SPECIALIZATIONS(macro, __VA_ARGS__)

#define PRO4D_GEN_DEBUG_SYMBOL_FOR_MEM_ACCESSOR(...)                           \
  PRO4D_DEBUG(accessor() noexcept { ::std::ignore = &accessor::__VA_ARGS__; })

#define PRO4D_EXPAND_IMPL(x) x

#define PRO4D_EXPAND_MACRO_IMPL(macro, _1, _2, _3, name, ...) macro##_##name
#define PRO4D_EXPAND_MACRO(macro, ...)                                         \
  PRO4D_EXPAND_IMPL(                                                           \
      PRO4D_EXPAND_MACRO_IMPL(macro, __VA_ARGS__, 3, 2)(__VA_ARGS__))

#define PRO4D_DEF_MEM_ACCESSOR(oq, pq, ne, ...)                                \
  template <class ProP, class ProD, class ProR, class... ProArgs>              \
  struct accessor<ProP, ProD, ProR(ProArgs...) oq ne> {                        \
    PRO4D_GEN_DEBUG_SYMBOL_FOR_MEM_ACCESSOR(__VA_ARGS__)                       \
    ProR __VA_ARGS__(ProArgs... pro_args) oq ne {                              \
      return ::pro::v4::proxy_invoke<ProD, ProR(ProArgs...) oq ne>(            \
          static_cast<ProP pq>(*this), ::std::forward<ProArgs>(pro_args)...);  \
    }                                                                          \
  }
#define PRO4D_DEF_MEM_DISPATCH_IMPL(name, impl, func)                          \
  struct name {                                                                \
    template <class ProT, class... ProArgs>                                    \
    PRO4D_STATIC_CALL(decltype(auto), ProT&& pro_self, ProArgs&&... pro_args)  \
    PRO4D_DIRECT_FUNC_IMPL(::std::forward<ProT>(pro_self).impl(                \
        ::std::forward<ProArgs>(pro_args)...))                                 \
        PRO4D_DEF_ACCESSOR_TEMPLATE(MEM, PRO4D_DEF_MEM_ACCESSOR, func)         \
  }
#define PRO4D_DEF_MEM_DISPATCH_2(name, impl)                                   \
  PRO4D_DEF_MEM_DISPATCH_IMPL(name, impl, impl)
#define PRO4D_DEF_MEM_DISPATCH_3(name, impl, func)                             \
  PRO4D_DEF_MEM_DISPATCH_IMPL(name, impl, func)
#define PRO4_DEF_MEM_DISPATCH(name, ...)                                       \
  PRO4D_EXPAND_MACRO(PRO4D_DEF_MEM_DISPATCH, name, __VA_ARGS__)

#define PRO4D_DEF_FREE_ACCESSOR(oq, pq, ne, ...)                               \
  template <class ProP, class ProD, class ProR, class... ProArgs>              \
  struct accessor<ProP, ProD, ProR(ProArgs...) oq ne> {                        \
    friend ProR __VA_ARGS__(ProP pq pro_self, ProArgs... pro_args) ne {        \
      return ::pro::v4::proxy_invoke<ProD, ProR(ProArgs...) oq ne>(            \
          static_cast<ProP pq>(pro_self),                                      \
          ::std::forward<ProArgs>(pro_args)...);                               \
    }                                                                          \
    PRO4D_DEBUG(                                                             \
      accessor() noexcept { ::std::ignore = &pro_symbol_guard; }             \
                                                                             \
    private:                                                                 \
      static inline ProR pro_symbol_guard(ProP pq pro_self,                  \
                                          ProArgs... pro_args) {             \
        return __VA_ARGS__(static_cast<ProP pq>(pro_self),                   \
            ::std::forward<ProArgs>(pro_args)...);                           \
      }                                                                      \
    ) \
  }
#define PRO4D_DEF_FREE_DISPATCH_IMPL(name, impl, func)                         \
  struct name {                                                                \
    template <class ProT, class... ProArgs>                                    \
    PRO4D_STATIC_CALL(decltype(auto), ProT&& pro_self, ProArgs&&... pro_args)  \
    PRO4D_DIRECT_FUNC_IMPL(impl(::std::forward<ProT>(pro_self),                \
                                ::std::forward<ProArgs>(pro_args)...))         \
        PRO4D_DEF_ACCESSOR_TEMPLATE(FREE, PRO4D_DEF_FREE_ACCESSOR, func)       \
  }
#define PRO4D_DEF_FREE_DISPATCH_2(name, impl)                                  \
  PRO4D_DEF_FREE_DISPATCH_IMPL(name, impl, impl)
#define PRO4D_DEF_FREE_DISPATCH_3(name, impl, func)                            \
  PRO4D_DEF_FREE_DISPATCH_IMPL(name, impl, func)
#define PRO4_DEF_FREE_DISPATCH(name, ...)                                      \
  PRO4D_EXPAND_MACRO(PRO4D_DEF_FREE_DISPATCH, name, __VA_ARGS__)

#define PRO4D_DEF_FREE_AS_MEM_DISPATCH_IMPL(name, impl, func)                  \
  struct name {                                                                \
    template <class ProT, class... ProArgs>                                    \
    PRO4D_STATIC_CALL(decltype(auto), ProT&& pro_self, ProArgs&&... pro_args)  \
    PRO4D_DIRECT_FUNC_IMPL(impl(::std::forward<ProT>(pro_self),                \
                                ::std::forward<ProArgs>(pro_args)...))         \
        PRO4D_DEF_ACCESSOR_TEMPLATE(MEM, PRO4D_DEF_MEM_ACCESSOR, func)         \
  }
#define PRO4D_DEF_FREE_AS_MEM_DISPATCH_2(name, impl)                           \
  PRO4D_DEF_FREE_AS_MEM_DISPATCH_IMPL(name, impl, impl)
#define PRO4D_DEF_FREE_AS_MEM_DISPATCH_3(name, impl, func)                     \
  PRO4D_DEF_FREE_AS_MEM_DISPATCH_IMPL(name, impl, func)
#define PRO4_DEF_FREE_AS_MEM_DISPATCH(name, ...)                               \
  PRO4D_EXPAND_MACRO(PRO4D_DEF_FREE_AS_MEM_DISPATCH, name, __VA_ARGS__)

// Version-less macro aliases

#define PRO4D_AMBIGUOUS_MACRO_DIAGNOSTIC_ASSERT(name, qualified_name)          \
  static_assert(false, "The use of macro `" #name "` is ambiguous. \
Are multiple different versions of Proxy library included at the same time?\n\
Note: To resolve this error: \n\
- Either make sure that only one version of Proxy library is included within this file.\n\
- Or use the `" #qualified_name "` macro (note the `4` suffix) to explicitly \
stick to a specific major version of the Proxy library.")

#ifdef __msft_lib_proxy
#undef __msft_lib_proxy
#define __msft_lib_proxy                                                       \
  [] {                                                                         \
    PRO4D_AMBIGUOUS_MACRO_DIAGNOSTIC_ASSERT(__msft_lib_proxy,                  \
                                            __msft_lib_proxy4);                \
    return 0L;                                                                 \
  }()
#else
#define __msft_lib_proxy __msft_lib_proxy4
#endif // __msft_lib_proxy

#ifdef PRO_DEF_MEM_DISPATCH
#undef PRO_DEF_MEM_DISPATCH
#define PRO_DEF_MEM_DISPATCH(...)                                              \
  PRO4D_AMBIGUOUS_MACRO_DIAGNOSTIC_ASSERT(PRO_DEF_MEM_DISPATCH,                \
                                          PRO4_DEF_MEM_DISPATCH)
#else
#define PRO_DEF_MEM_DISPATCH(name, ...) PRO4_DEF_MEM_DISPATCH(name, __VA_ARGS__)
#endif // PRO_DEF_MEM_DISPATCH

#ifdef PRO_DEF_FREE_DISPATCH
#undef PRO_DEF_FREE_DISPATCH
#define PRO_DEF_FREE_DISPATCH(...)                                             \
  PRO4D_AMBIGUOUS_MACRO_DIAGNOSTIC_ASSERT(PRO_DEF_FREE_DISPATCH,               \
                                          PRO4_DEF_FREE_DISPATCH)
#else
#define PRO_DEF_FREE_DISPATCH(name, ...)                                       \
  PRO4_DEF_FREE_DISPATCH(name, __VA_ARGS__)
#endif // PRO_DEF_FREE_DISPATCH

#ifdef PRO_DEF_FREE_AS_MEM_DISPATCH
#undef PRO_DEF_FREE_AS_MEM_DISPATCH
#define PRO_DEF_FREE_AS_MEM_DISPATCH(...)                                      \
  PRO4D_AMBIGUOUS_MACRO_DIAGNOSTIC_ASSERT(PRO_DEF_FREE_AS_MEM_DISPATCH,        \
                                          PRO4_DEF_FREE_AS_MEM_DISPATCH)
#else
#define PRO_DEF_FREE_AS_MEM_DISPATCH(name, ...)                                \
  PRO4_DEF_FREE_AS_MEM_DISPATCH(name, __VA_ARGS__)
#endif // PRO_DEF_FREE_AS_MEM_DISPATCH

#endif // MSFT_PROXY_V4_PROXY_MACROS_H_
