// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#ifndef _MSFT_PROXY4_MACROS_
#define _MSFT_PROXY4_MACROS_

#if __cpp_static_call_operator >= 202207L
#define ___PRO4_STATIC_CALL(__R, ...) static __R operator()(__VA_ARGS__)
#else
#define ___PRO4_STATIC_CALL(__R, ...) __R operator()(__VA_ARGS__) const
#endif // __cpp_static_call_operator >= 202207L

#ifdef _MSC_VER
#define ___PRO4_ENFORCE_EBO __declspec(empty_bases)
#else
#define ___PRO4_ENFORCE_EBO
#endif // _MSC_VER

#ifdef NDEBUG
#define ___PRO4_DEBUG(...)
#else
#define ___PRO4_DEBUG(...) __VA_ARGS__
#endif // NDEBUG

#define __msft_lib_proxy4 202503L

////////

#define ___PRO4_DIRECT_FUNC_IMPL(...)                                          \
  noexcept(noexcept(__VA_ARGS__))                                              \
    requires(requires { __VA_ARGS__; })                                        \
  {                                                                            \
    return __VA_ARGS__;                                                        \
  }

#define ___PRO4_DEF_MEM_ACCESSOR_TEMPLATE(__MACRO, ...)                        \
  template <class __F, bool __IsDirect, class __D, class... __Os>              \
  struct ___PRO4_ENFORCE_EBO accessor {                                        \
    accessor() = delete;                                                       \
  };                                                                           \
  template <class __F, bool __IsDirect, class __D, class... __Os>              \
    requires(                                                                  \
        sizeof...(__Os) > 1u &&                                                \
        (::std::is_constructible_v<accessor<__F, __IsDirect, __D, __Os>> &&    \
         ...))                                                                 \
  struct accessor<__F, __IsDirect, __D, __Os...>                               \
      : accessor<__F, __IsDirect, __D, __Os>... {                              \
    using accessor<__F, __IsDirect, __D, __Os>::__VA_ARGS__...;                \
  };                                                                           \
  __MACRO(, ::pro::v4::access_proxy<__F>(*this), __VA_ARGS__);                 \
  __MACRO(noexcept, ::pro::v4::access_proxy<__F>(*this), __VA_ARGS__);         \
  __MACRO(&, ::pro::v4::access_proxy<__F>(*this), __VA_ARGS__);                \
  __MACRO(& noexcept, ::pro::v4::access_proxy<__F>(*this), __VA_ARGS__);       \
  __MACRO(&&, ::pro::v4::access_proxy<__F>(::std::move(*this)), __VA_ARGS__);  \
  __MACRO(&& noexcept, ::pro::v4::access_proxy<__F>(::std::move(*this)),       \
          __VA_ARGS__);                                                        \
  __MACRO(const, ::pro::v4::access_proxy<__F>(*this), __VA_ARGS__);            \
  __MACRO(const noexcept, ::pro::v4::access_proxy<__F>(*this), __VA_ARGS__);   \
  __MACRO(const&, ::pro::v4::access_proxy<__F>(*this), __VA_ARGS__);           \
  __MACRO(const& noexcept, ::pro::v4::access_proxy<__F>(*this), __VA_ARGS__);  \
  __MACRO(const&&, ::pro::v4::access_proxy<__F>(::std::move(*this)),           \
          __VA_ARGS__);                                                        \
  __MACRO(const&& noexcept, ::pro::v4::access_proxy<__F>(::std::move(*this)),  \
          __VA_ARGS__);

#define ___PRO4_ADL_ARG ::pro::v4::details::adl_accessor_arg_t<__F, __IsDirect>
#define ___PRO4_DEF_FREE_ACCESSOR_TEMPLATE(__MACRO, ...)                       \
  template <class __F, bool __IsDirect, class __D, class... __Os>              \
  struct ___PRO4_ENFORCE_EBO accessor {                                        \
    accessor() = delete;                                                       \
  };                                                                           \
  template <class __F, bool __IsDirect, class __D, class... __Os>              \
    requires(                                                                  \
        sizeof...(__Os) > 1u &&                                                \
        (::std::is_constructible_v<accessor<__F, __IsDirect, __D, __Os>> &&    \
         ...))                                                                 \
  struct accessor<__F, __IsDirect, __D, __Os...>                               \
      : accessor<__F, __IsDirect, __D, __Os>... {};                            \
  __MACRO(, , ___PRO4_ADL_ARG& __self, ::pro::v4::access_proxy<__F>(__self),   \
          __VA_ARGS__);                                                        \
  __MACRO(noexcept, noexcept, ___PRO4_ADL_ARG& __self,                         \
          ::pro::v4::access_proxy<__F>(__self), __VA_ARGS__);                  \
  __MACRO(&, , ___PRO4_ADL_ARG& __self, ::pro::v4::access_proxy<__F>(__self),  \
          __VA_ARGS__);                                                        \
  __MACRO(& noexcept, noexcept, ___PRO4_ADL_ARG& __self,                       \
          ::pro::v4::access_proxy<__F>(__self), __VA_ARGS__);                  \
  __MACRO(&&, , ___PRO4_ADL_ARG&& __self,                                      \
          ::pro::v4::access_proxy<__F>(                                        \
              ::std::forward<decltype(__self)>(__self)),                       \
          __VA_ARGS__);                                                        \
  __MACRO(&& noexcept, noexcept, ___PRO4_ADL_ARG&& __self,                     \
          ::pro::v4::access_proxy<__F>(                                        \
              ::std::forward<decltype(__self)>(__self)),                       \
          __VA_ARGS__);                                                        \
  __MACRO(const, , const ___PRO4_ADL_ARG& __self,                              \
          ::pro::v4::access_proxy<__F>(__self), __VA_ARGS__);                  \
  __MACRO(const noexcept, noexcept, const ___PRO4_ADL_ARG& __self,             \
          ::pro::v4::access_proxy<__F>(__self), __VA_ARGS__);                  \
  __MACRO(const&, , const ___PRO4_ADL_ARG& __self,                             \
          ::pro::v4::access_proxy<__F>(__self), __VA_ARGS__);                  \
  __MACRO(const& noexcept, noexcept, const ___PRO4_ADL_ARG& __self,            \
          ::pro::v4::access_proxy<__F>(__self), __VA_ARGS__);                  \
  __MACRO(                                                                     \
      const&&, , const ___PRO4_ADL_ARG&& __self,                               \
      ::pro::v4::access_proxy<__F>(::std::forward<decltype(__self)>(__self)),  \
      __VA_ARGS__);                                                            \
  __MACRO(                                                                     \
      const&& noexcept, noexcept, const ___PRO4_ADL_ARG&& __self,              \
      ::pro::v4::access_proxy<__F>(::std::forward<decltype(__self)>(__self)),  \
      __VA_ARGS__);

#define ___PRO4_GEN_DEBUG_SYMBOL_FOR_MEM_ACCESSOR(...)                         \
  ___PRO4_DEBUG(accessor() noexcept { ::std::ignore = &accessor::__VA_ARGS__; })

#define ___PRO4_EXPAND_IMPL(__X) __X

#define ___PRO4_EXPAND_MACRO_IMPL(__MACRO, __1, __2, __3, __NAME, ...)         \
  __MACRO##_##__NAME
#define ___PRO4_EXPAND_MACRO(__MACRO, ...)                                     \
  ___PRO4_EXPAND_IMPL(                                                         \
      ___PRO4_EXPAND_MACRO_IMPL(__MACRO, __VA_ARGS__, 3, 2)(__VA_ARGS__))

////////

#define ___PRO4_DEF_MEM_ACCESSOR(__Q, __SELF, ...)                             \
  template <class __F, bool __IsDirect, class __D, class __R, class... __Args> \
  struct accessor<__F, __IsDirect, __D, __R(__Args...) __Q> {                  \
    ___PRO4_GEN_DEBUG_SYMBOL_FOR_MEM_ACCESSOR(__VA_ARGS__)                     \
    __R __VA_ARGS__(__Args... __args) __Q {                                    \
      return ::pro::v4::proxy_invoke<__IsDirect, __D, __R(__Args...) __Q>(     \
          __SELF, ::std::forward<__Args>(__args)...);                          \
    }                                                                          \
  }
#define ___PRO4_DEF_MEM_DISPATCH_IMPL(__NAME, __FUNC, __FNAME, __TTYPE)        \
  struct __NAME {                                                              \
    template <__TTYPE __T, class... __Args>                                    \
    ___PRO4_STATIC_CALL(decltype(auto), __T&& __self, __Args&&... __args)      \
    ___PRO4_DIRECT_FUNC_IMPL(                                                  \
        ::std::forward<__T>(__self).__FUNC(::std::forward<__Args>(__args)...)) \
        ___PRO4_DEF_MEM_ACCESSOR_TEMPLATE(___PRO4_DEF_MEM_ACCESSOR, __FNAME)   \
  }
#define ___PRO4_DEF_MEM_DISPATCH_2(__NAME, __FUNC)                             \
  ___PRO4_DEF_MEM_DISPATCH_IMPL(__NAME, __FUNC, __FUNC,                        \
                                ::pro::v4::details::non_proxy_arg)
#define ___PRO4_DEF_MEM_DISPATCH_3(__NAME, __FUNC, __FNAME)                    \
  ___PRO4_DEF_MEM_DISPATCH_IMPL(__NAME, __FUNC, __FNAME, class)
#define PRO4_DEF_MEM_DISPATCH(__NAME, ...)                                     \
  ___PRO4_EXPAND_MACRO(___PRO4_DEF_MEM_DISPATCH, __NAME, __VA_ARGS__)

#define ___PRO4_DEF_FREE_ACCESSOR(__Q, __NE, __SELF_ARG, __SELF, ...)          \
  template <class __F, bool __IsDirect, class __D, class __R, class... __Args> \
  struct accessor<__F, __IsDirect, __D, __R(__Args...) __Q> {                  \
    friend __R __VA_ARGS__(__SELF_ARG, __Args... __args) __NE {                \
      return ::pro::v4::proxy_invoke<__IsDirect, __D, __R(__Args...) __Q>(     \
          __SELF, ::std::forward<__Args>(__args)...);                          \
    }                                                                          \
    ___PRO4_DEBUG(                                                         \
      accessor() noexcept { ::std::ignore = &_symbol_guard; }              \
                                                                           \
    private:                                                               \
      static inline __R _symbol_guard(__SELF_ARG, __Args... __args) __NE { \
        return __VA_ARGS__(::std::forward<decltype(__self)>(__self),       \
            ::std::forward<__Args>(__args)...);                            \
      }                                                                    \
    )   \
  }
#define ___PRO4_DEF_FREE_DISPATCH_IMPL(__NAME, __FUNC, __FNAME, __TTYPE)       \
  struct __NAME {                                                              \
    template <__TTYPE __T, class... __Args>                                    \
    ___PRO4_STATIC_CALL(decltype(auto), __T&& __self, __Args&&... __args)      \
    ___PRO4_DIRECT_FUNC_IMPL(__FUNC(::std::forward<__T>(__self),               \
                                    ::std::forward<__Args>(__args)...))        \
        ___PRO4_DEF_FREE_ACCESSOR_TEMPLATE(___PRO4_DEF_FREE_ACCESSOR, __FNAME) \
  }
#define ___PRO4_DEF_FREE_DISPATCH_2(__NAME, __FUNC)                            \
  ___PRO4_DEF_FREE_DISPATCH_IMPL(__NAME, __FUNC, __FUNC,                       \
                                 ::pro::v4::details::non_proxy_arg)
#define ___PRO4_DEF_FREE_DISPATCH_3(__NAME, __FUNC, __FNAME)                   \
  ___PRO4_DEF_FREE_DISPATCH_IMPL(__NAME, __FUNC, __FNAME, class)
#define PRO4_DEF_FREE_DISPATCH(__NAME, ...)                                    \
  ___PRO4_EXPAND_MACRO(___PRO4_DEF_FREE_DISPATCH, __NAME, __VA_ARGS__)

#define ___PRO4_DEF_FREE_AS_MEM_DISPATCH_IMPL(__NAME, __FUNC, __FNAME)         \
  struct __NAME {                                                              \
    template <class __T, class... __Args>                                      \
    ___PRO4_STATIC_CALL(decltype(auto), __T&& __self, __Args&&... __args)      \
    ___PRO4_DIRECT_FUNC_IMPL(__FUNC(::std::forward<__T>(__self),               \
                                    ::std::forward<__Args>(__args)...))        \
        ___PRO4_DEF_MEM_ACCESSOR_TEMPLATE(___PRO4_DEF_MEM_ACCESSOR, __FNAME)   \
  }
#define ___PRO4_DEF_FREE_AS_MEM_DISPATCH_2(__NAME, __FUNC)                     \
  ___PRO4_DEF_FREE_AS_MEM_DISPATCH_IMPL(__NAME, __FUNC, __FUNC)
#define ___PRO4_DEF_FREE_AS_MEM_DISPATCH_3(__NAME, __FUNC, __FNAME)            \
  ___PRO4_DEF_FREE_AS_MEM_DISPATCH_IMPL(__NAME, __FUNC, __FNAME)
#define PRO4_DEF_FREE_AS_MEM_DISPATCH(__NAME, ...)                             \
  ___PRO4_EXPAND_MACRO(___PRO4_DEF_FREE_AS_MEM_DISPATCH, __NAME, __VA_ARGS__)

// Version-less macro aliases

#define ___PRO4_AMBIGUOUS_MACRO_DIAGNOSTIC_ASSERT(__NAME,                      \
                                                  __VERSION_QUALIFIED_NAME)    \
  static_assert(false, "The use of macro `" #__NAME "` is ambiguous. \
Are multiple different versions of Proxy library included at the same time?\n\
Note: To resolve this error: \n\
- Either make sure that only one version of Proxy library is included within this file.\n\
- Or use the `" #__VERSION_QUALIFIED_NAME                                      \
                       "` macro (note the `4` suffix) to explicitly \
stick to a specific major version of the Proxy library.")

#ifdef __msft_lib_proxy
#undef __msft_lib_proxy
#define __msft_lib_proxy                                                       \
  [] {                                                                         \
    ___PRO4_AMBIGUOUS_MACRO_DIAGNOSTIC_ASSERT(__msft_lib_proxy,                \
                                              __msft_lib_proxy4);              \
    return 0L;                                                                 \
  }()
#else
#define __msft_lib_proxy __msft_lib_proxy4
#endif // __msft_lib_proxy

#ifdef PRO_DEF_MEM_DISPATCH
#undef PRO_DEF_MEM_DISPATCH
#define PRO_DEF_MEM_DISPATCH(...)                                              \
  ___PRO4_AMBIGUOUS_MACRO_DIAGNOSTIC_ASSERT(PRO_DEF_MEM_DISPATCH,              \
                                            PRO4_DEF_MEM_DISPATCH)
#else
#define PRO_DEF_MEM_DISPATCH(name, ...) PRO4_DEF_MEM_DISPATCH(name, __VA_ARGS__)
#endif // PRO_DEF_MEM_DISPATCH

#ifdef PRO_DEF_FREE_DISPATCH
#undef PRO_DEF_FREE_DISPATCH
#define PRO_DEF_FREE_DISPATCH(...)                                             \
  ___PRO4_AMBIGUOUS_MACRO_DIAGNOSTIC_ASSERT(PRO_DEF_FREE_DISPATCH,             \
                                            PRO4_DEF_FREE_DISPATCH)
#else
#define PRO_DEF_FREE_DISPATCH(name, ...)                                       \
  PRO4_DEF_FREE_DISPATCH(name, __VA_ARGS__)
#endif // PRO_DEF_FREE_DISPATCH

#ifdef PRO_DEF_FREE_AS_MEM_DISPATCH
#undef PRO_DEF_FREE_AS_MEM_DISPATCH
#define PRO_DEF_FREE_AS_MEM_DISPATCH(...)                                      \
  ___PRO4_AMBIGUOUS_MACRO_DIAGNOSTIC_ASSERT(PRO_DEF_FREE_AS_MEM_DISPATCH,      \
                                            PRO4_DEF_FREE_AS_MEM_DISPATCH)
#else
#define PRO_DEF_FREE_AS_MEM_DISPATCH(name, ...)                                \
  PRO4_DEF_FREE_AS_MEM_DISPATCH(name, __VA_ARGS__)
#endif // PRO_DEF_FREE_AS_MEM_DISPATCH

#endif // _MSFT_PROXY4_MACROS_
