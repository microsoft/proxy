# Macro `PRO_DEF_FREE_DISPATCH`

```cpp
#define PRO_DEF_FREE_DISPATCH // see below
```

Macro `PRO_DEF_FREE_DISPATCH` defines dispatch types for free function expressions with accessibility. It supports two syntaxes:

```cpp
// (1)
PRO_DEF_FREE_DISPATCH(dispatch_name, func_name);

// (2)
PRO_DEF_FREE_DISPATCH(dispatch_name, func_name, accessibility_func_name);
```

`(1)` Equivalent to `PRO_DEF_FREE_DISPATCH(dispatch_name, func_name, func_name);`

`(2)` Defines a class named `dispatch_name` of free function call expressions of `func_name` with accessibility. `dispatch_name` meets the [*ProAccessible*](ProAccessible.md) requirements of types `F`, `C`, and `Os...`, where `F` models concept [`facade`](facade.md), `C` is a tuple element type defined in `typename F::convention_types`, and each type `O` (possibly qualified with *cv ref noex*) in `Os...` is a tuple element type defined in `typename C::overload_types`. Let `accessor_arg` be `std::conditional_t<C::is_direct, proxy<F>, proxy_indirect_accessor<F>>`. The functions provided by `typename dispatch_name::template accessor<F, C, Os...>` are named `accessibility_func_name` and can be found by [argument-dependent lookup](https://en.cppreference.com/w/cpp/language/adl) when `accessor_arg` is an associated class of the arguments. Effectively equivalent to:

```cpp
struct dispatch_name {
  template <class T, class... Args>
  decltype(auto) operator()(T&& self, Args&&... args)
      noexcept(noexcept(func_name(std::forward<T>(self), std::forward<Args>(args)...)))
      requires(requires { func_name(std::forward<T>(self), std::forward<Args>(args)...); }) {
    return func_name(std::forward<T>(self), std::forward<Args>(args)...);
  }

  template <class F, class C, class... Os> struct accessor {
    accessor() = delete;
  };
  template <class F, class C, class... Os>
      requires(sizeof...(Os) > 1u && (std::is_constructible_v<accessor<F, C, Os>> && ...))
  struct accessor<F, C, Os...> : accessor<F, C, Os>... {};
  template <class F, class C, class R, class... Args>
  struct accessor<F, C, R(Args...) cv ref noex> {
    friend R accessibility_func_name(accessor_arg cv ref self, Args... args) noex {
      return pro::proxy_invoke<C, R(Args...) cv ref noex>(pro::access_proxy<F>(std::forward<accessor_arg cv ref>(self)), std::forward<Args>(args)...);
    }
  };
}
```

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
  std::cout << ToString(*p) << "\n";  // Prints: "123"
}
```

## See Also

- [macro `PRO_DEF_MEM_DISPATCH`](PRO_DEF_MEM_DISPATCH.md)
- [macro `PRO_DEF_FREE_AS_MEM_DISPATCH`](PRO_DEF_FREE_AS_MEM_DISPATCH.md)
- [alias template `basic_facade_builder::add_convention`](basic_facade_builder/add_convention.md)
