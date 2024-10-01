# Function template `proxy_invoke`

```cpp
template <class C, class O, class F, class... Args>
/* see below */ proxy_invoke(proxy<F>& p, Args&&... args);

template <class C, class O, class F, class... Args>
/* see below */ proxy_invoke(const proxy<F>& p, Args&&... args);

template <class C, class O, class F, class... Args>
/* see below */ proxy_invoke(proxy<F>&& p, Args&&... args);

template <class C, class O, class F, class... Args>
/* see below */ proxy_invoke(const proxy<F>&& p, Args&&... args);
```

Invokes a `proxy` with a specified convention type, an overload type, and arguments. `C` is required to be defined in `typename F::convention_types`. `O` is required to be defined in `typename C::overload_types`.

Let `ptr` be the contained value of `p` with the same cv ref-qualifiers, `Args2...` be the argument types of `O`, `R` be the return type of `O`,

- if `C::is_direct` is `true`, let `v` be `std::forward<decltype(ptr)>(ptr)`, or otherwise,
- if `C::is_direct` is `false`, let `v` be `*std::forward<decltype(ptr)>(ptr)`,

equivalent to:

- [`INVOKE<R>`](https://en.cppreference.com/w/cpp/utility/functional)`(typename C::dispatch_type{}, std::forward<decltype(v)>(v), static_cast<Args2>(args)...)` if the expression is well-formed, or otherwise,
- [`INVOKE<R>`](https://en.cppreference.com/w/cpp/utility/functional)`(typename C::dispatch_type{}, nullptr, static_cast<Args2>(args)...)`.

The behavior is undefined if `p` does not contain a value.

## Notes

It is generally not recommended to call `proxy_invoke` directly. Using an [`accessor`](ProAccessible.md) is usually a better option with easier and more descriptive syntax. If the facade type `F` is defined with the recommended facilities, it has full accessibility support. Specifically, when

- the underlying dispatch type `typename C::dispatch_type` is defined via [macro `PRO_DEF_MEM_DISPATCH`](PRO_DEF_MEM_DISPATCH.md), [macro `PRO_DEF_FREE_DISPATCH`](PRO_DEF_FREE_DISPATCH.md), or is a specialization of either [`operator_dispatch`](operator_dispatch.md) or [`conversion_dispatch`](conversion_dispatch.md), and
- the convention is defined via [`facade_builder`](basic_facade_builder.md).

## Example

```cpp
#include <iostream>
#include <string>

#include "proxy.h"

PRO_DEF_FREE_DISPATCH(FreeToString, std::to_string, ToString);

struct Stringable : pro::facade_builder
    ::add_convention<FreeToString, std::string() const>
    ::build {};

int main() {
  int a = 123;
  pro::proxy<Stringable> p = &a;
  std::cout << ToString(*p) << "\n";  // Invokes with accessor, prints: "123"

  using C = std::tuple_element_t<0u, Stringable::convention_types>;
  std::cout << pro::proxy_invoke<C, std::string() const>(p) << "\n";  // Invokes with proxy_invoke, also prints: "123"
}
```

## See Also

- [function template `access_proxy`](access_proxy.md)
- [function template `proxy_reflect`](proxy_reflect.md)
