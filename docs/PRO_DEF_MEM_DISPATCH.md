# Macro `PRO_DEF_MEM_DISPATCH`

```cpp
#define PRO_DEF_MEM_DISPATCH // see below
```

Macro `PRO_DEF_MEM_DISPATCH` defines dispatch types for member function expressions with accessibility. It supports two syntaxes:

```cpp
// (1)
PRO_DEF_MEM_DISPATCH(dispatch_name, func_name);

// (2)
PRO_DEF_MEM_DISPATCH(dispatch_name, func_name, accessibility_func_name);
```

`(1)` Equivalent to `PRO_DEF_MEM_DISPATCH(dispatch_name, func_name, func_name);`

`(2)` Defines a class named `dispatch_name` of member function call expressions of `func_name` with accessibility. `dispatch_name` meets the [*ProAccessible*](ProAccessible.md) requirements of types `F`, `C`, and `Os...`, where `F` models concept [`facade`](facade.md), `C` is a tuple element type defined in `typename F::convention_types`, and each type `O` (possibly qualified with *cv, ref, noex*) in `Os...` is a tuple element type defined in `typename C::overload_types`. The member functions provided by `typename dispatch_name::template accessor<F, C, Os...>` are named `accessibility_func_name`. Let `SELF` be `std::forward<accessor cv ref>(*this)`, effectively equivalent to:

```cpp
struct dispatch_name {
  template <class T, class... Args>
  decltype(auto) operator()(T&& self, Args&&... args)
      noexcept(noexcept(std::forward<T>(self).func_name(std::forward<Args>(args)...)))
      requires(requires { std::forward<T>(self).func_name(std::forward<Args>(args)...); }) {
    return std::forward<T>(self).func_name(std::forward<Args>(args)...);
  }

  template <class F, class C, class... Os>
  struct accessor {
    accessor() = delete;
  };
  template <class F, class C, class... Os>
      requires(sizeof...(Os) > 1u && (std::is_trivial_v<accessor<F, C, Os>> && ...))
  struct accessor<F, C, Os...> : accessor<F, C, Os>... {
    using accessor<F, C, Os>::accessibility_func_name ...;
  };
  template <class F, class C, class R, class... Args>
  struct accessor<F, C, R(Args...) cv ref noex> {
    R accessibility_func_name(Args... args) cv ref noex {
      return pro::proxy_invoke<C, R(Args...) cv ref noex>(pro::access_proxy<F>(SELF), std::forward<Args>(args)...);
    }
  };
}
```

## Example

```cpp
#include <iostream>
#include <string>
#include <vector>

#include "proxy.h"

PRO_DEF_MEM_DISPATCH(MemAt, at);

struct Dictionary : pro::facade_builder
    ::add_convention<MemAt, std::string(int index) const>
    ::build {};

int main() {
  std::vector<const char*> v{"hello", "world"};
  pro::proxy<Dictionary> p = &v;
  std::cout << p->at(1) << "\n";  // Prints: "world"
}
```

## See Also

- [macro `PRO_DEF_FREE_DISPATCH`](PRO_DEF_FREE_DISPATCH.md)
- [alias template `basic_facade_builder::add_convention`](basic_facade_builder/add_convention.md)
