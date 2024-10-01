# Function template `access_proxy`

```cpp
template <class F, class A>
proxy<F>& access_proxy(A& a) noexcept;

template <class F, class A>
const proxy<F>& access_proxy(const A& a) noexcept;

template <class F, class A>
proxy<F>&& access_proxy(A&& a) noexcept;

template <class F, class A>
const proxy<F>&& access_proxy(const A&& a) noexcept;
```

Accesses a `proxy` object from an [accessor](ProAccessible.md) instantiated from the `proxy`. `F` shall model concept [`facade`](facade.md). As per `facade<F>`, `typename F::convention_types` shall be a [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type containing distinct types `Cs`. There shall be a type `C` in `Cs` where `A` is the same type as `typename C::template accessor<F>`. The behavior is undefined if `a` is not instantiated from a `proxy`.

## Return Value

A reference to the `proxy` that instantiates `a`.

## Notes

Similar to [`proxy_invoke`](proxy_invoke.md), this function can be used to implement the accessibility of `proxy`. If the facade type `F` is defined with the recommended facilities, it has full accessibility support. Specifically, when:

- the underlying dispatch type `typename C::dispatch_type` is defined via [macro `PRO_DEF_MEM_DISPATCH`](PRO_DEF_MEM_DISPATCH.md), [macro `PRO_DEF_FREE_DISPATCH`](PRO_DEF_FREE_DISPATCH.md), or is a specialization of either [`operator_dispatch`](operator_dispatch.md) or [`conversion_dispatch`](conversion_dispatch.md), and
- the convention is defined via [`facade_builder`](basic_facade_builder.md).

## Example

```cpp
#include <iostream>
#include <string>

#include "proxy.h"

PRO_DEF_FREE_DISPATCH(FreeToString, std::to_string, ToString);

struct Stringable : pro::facade_builder
    ::add_convention<FreeToString, std::string()>
    ::build {};

int main() {
  pro::proxy<Stringable> p = pro::make_proxy<Stringable>(123);

  // Invokes with accessibility API
  std::cout << ToString(*p) << "\n";  // Prints: "123"

  // How it works behind the scenes
  using Convention = std::tuple_element_t<0u, Stringable::convention_types>;
  using Accessor = Convention::accessor<Stringable>;
  static_assert(std::is_base_of_v<Accessor, std::remove_reference_t<decltype(*p)>>);
  Accessor& a = static_cast<Accessor&>(*p);
  pro::proxy<Stringable>& p2 = pro::access_proxy<Stringable>(a);
  std::cout << std::boolalpha << (&p == &p2) << "\n";  // Prints: "true" because access_proxy converts
                                                       // an accessor back to the original proxy
  auto result = pro::proxy_invoke<Convention, std::string()>(p2);
  std::cout << result << "\n";  // Prints: "123"
}
```

## See Also

- [named requirements *ProAccessible*](ProAccessible.md)
- [function template `proxy_invoke`](proxy_invoke.md)
