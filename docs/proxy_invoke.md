# Function template `proxy_invoke`

```cpp
template <bool IsDirect, class D, class O, class F, class... Args>
/* see below */ proxy_invoke(proxy<F>& p, Args&&... args);

template <bool IsDirect, class D, class O, class F, class... Args>
/* see below */ proxy_invoke(const proxy<F>& p, Args&&... args);

template <bool IsDirect, class D, class O, class F, class... Args>
/* see below */ proxy_invoke(proxy<F>&& p, Args&&... args);

template <bool IsDirect, class D, class O, class F, class... Args>
/* see below */ proxy_invoke(const proxy<F>&& p, Args&&... args);
```

Invokes a `proxy` with a specified dispatch type, an overload type, and arguments. There shall be a convention type `Conv` defined in `typename F::convention_types` where `Conv::is_direct == IsDirect && std::is_same_v<typename Conv::dispatch_type, D>` is `true`. `O` is required to be defined in `typename Conv::overload_types`.

Let `ptr` be the contained value of `p` with the same cv ref-qualifiers, `Args2...` be the argument types of `O`, `R` be the return type of `O`,

- if `IsDirect` is `true`, let `v` be `std::forward<decltype(ptr)>(ptr)`, or otherwise,
- if `IsDirect` is `false`, let `v` be `*std::forward<decltype(ptr)>(ptr)`,

equivalent to:

- [`INVOKE<R>`](https://en.cppreference.com/w/cpp/utility/functional)`(D{}, std::forward<decltype(v)>(v), static_cast<Args2>(args)...)` if the expression is well-formed, or otherwise,
- [`INVOKE<R>`](https://en.cppreference.com/w/cpp/utility/functional)`(D{}, nullptr, static_cast<Args2>(args)...)`.

The behavior is undefined if `p` does not contain a value.

## Notes

It is generally not recommended to call `proxy_invoke` directly. Using an [`accessor`](ProAccessible.md) is usually a better option with easier and more descriptive syntax. If the facade type `F` is defined with the recommended facilities, it has full accessibility support. Specifically, when

- `D` is defined via [macro `PRO_DEF_MEM_DISPATCH`](PRO_DEF_MEM_DISPATCH.md), [macro `PRO_DEF_FREE_DISPATCH`](PRO_DEF_FREE_DISPATCH.md), or is a specialization of either [`operator_dispatch`](operator_dispatch.md) or [`conversion_dispatch`](conversion_dispatch.md), and
- the convention type `Conv` is defined via [`facade_builder`](basic_facade_builder.md).

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
  std::cout << pro::proxy_invoke<false, FreeToString, std::string() const>(p) << "\n";  // Invokes with proxy_invoke, also prints: "123"
}
```

## See Also

- [function template `access_proxy`](access_proxy.md)
- [function template `proxy_reflect`](proxy_reflect.md)
