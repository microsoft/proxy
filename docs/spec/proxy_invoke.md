# Function template `proxy_invoke`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v4`

```cpp
// (1)
template <class D, class O, facade F, class... Args>
return-type-of<O> proxy_invoke(proxy_indirect_accessor<F>& p, Args&&... args);
template <class D, class O, facade F, class... Args>
return-type-of<O> proxy_invoke(const proxy_indirect_accessor<F>& p, Args&&... args);
template <class D, class O, facade F, class... Args>
return-type-of<O> proxy_invoke(proxy_indirect_accessor<F>&& p, Args&&... args);
template <class D, class O, facade F, class... Args>
return-type-of<O> proxy_invoke(const proxy_indirect_accessor<F>&& p, Args&&... args);

// (2)
template <class D, class O, facade F, class... Args>
return-type-of<O> proxy_invoke(proxy<F>& p, Args&&... args);
template <class D, class O, facade F, class... Args>
return-type-of<O> proxy_invoke(const proxy<F>& p, Args&&... args);
template <class D, class O, facade F, class... Args>
return-type-of<O> proxy_invoke(proxy<F>&& p, Args&&... args);
template <class D, class O, facade F, class... Args>
return-type-of<O> proxy_invoke(const proxy<F>&& p, Args&&... args);
```

Invokes a `proxy` with a specified dispatch type, an overload type, and arguments. Let `Args2...` be the argument types of `O`, `R` be the return type of `O`. `return-type-of<O>` is `R`.

- `(1)` Let `ptr` be the contained value of the `proxy` object associated to `p` with the same cv ref-qualifiers. Equivalent to [`INVOKE<R>`](https://en.cppreference.com/w/cpp/utility/functional)`(D(), *ptr, static_cast<Args2>(args)...)`.
- `(2)` Let `ptr` be the contained value of `p` with the same cv ref-qualifiers. Equivalent to [`INVOKE<R>`](https://en.cppreference.com/w/cpp/utility/functional)`(D(), ptr, static_cast<Args2>(args)...)`.  The behavior is undefined if `p` does not contain a value.

There shall be a convention type `Conv` defined in `typename F::convention_types` where

- `Conv::is_direct` is `false` (for `(1)`) or `true` (for `(2)`), and
- `typename Conv::dispatch_type` is `D`, and
- there shall be an overload type `O1` defined in `typename Conv::overload_types` where [`substituted-overload`](ProOverload.md)`<O1, F>` is `O`.

## Notes

It is generally not recommended to call `proxy_invoke` directly. Using an [`accessor`](ProAccessible.md) is usually a better option with easier and more descriptive syntax. If the facade type `F` is defined with the recommended facilities, it has full accessibility support. Specifically, when

- `D` is defined via [macro `PRO_DEF_MEM_DISPATCH`](PRO_DEF_MEM_DISPATCH.md), [macro `PRO_DEF_FREE_DISPATCH`](PRO_DEF_FREE_DISPATCH.md), or is a specialization of either [`operator_dispatch`](operator_dispatch/README.md) or [`conversion_dispatch`](explicit_conversion_dispatch/README.md), and
- the convention type `Conv` is defined via [`facade_builder`](basic_facade_builder/README.md).

## Example

```cpp
#include <iostream>
#include <string>

#include <proxy/proxy.h>

PRO_DEF_FREE_DISPATCH(FreeToString, std::to_string, ToString);

struct Stringable : pro::facade_builder                                 //
                    ::add_convention<FreeToString, std::string() const> //
                    ::build {};

int main() {
  int a = 123;
  pro::proxy<Stringable> p = &a;
  std::cout << ToString(*p) << "\n"; // Invokes with accessor, prints: "123"
  std::cout << pro::proxy_invoke<FreeToString, std::string() const>(*p)
            << "\n"; // Invokes with proxy_invoke, also prints: "123"
}
```

## See Also

- [function template `proxy_reflect`](proxy_reflect.md)
