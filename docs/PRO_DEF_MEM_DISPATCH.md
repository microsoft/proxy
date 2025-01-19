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

`(2)` Defines a class named `dispatch_name` of member function call expressions of `func_name` with accessibility via member function overloads named `accessibility_func_name`. Effectively equivalent to:

```cpp
struct dispatch_name {
  template <class T, class... Args>
  decltype(auto) operator()(T&& self, Args&&... args) const
      noexcept(noexcept(std::forward<T>(self).func_name(std::forward<Args>(args)...)))
      requires(requires { std::forward<T>(self).func_name(std::forward<Args>(args)...); }) {
    return std::forward<T>(self).func_name(std::forward<Args>(args)...);
  }

  template <class F, bool IsDirect, class D, class... Os>
  struct accessor {
    accessor() = delete;
  };
  template <class F, bool IsDirect, class D, class... Os>
      requires(sizeof...(Os) > 1u && (std::is_constructible_v<accessor<F, IsDirect, D, Os>> && ...))
  struct accessor<F, IsDirect, D, Os...> : accessor<F, IsDirect, D, Os>... {
    using accessor<F, IsDirect, D, Os>::accessibility_func_name ...;
  };
  template <class F, bool IsDirect, class D, class R, class... Args>
  struct accessor<F, IsDirect, D, R(Args...) cv ref noex> {
    R accessibility_func_name(Args... args) cv ref noex {
      return pro::proxy_invoke<IsDirect, D, R(Args...) cv ref noex>(pro::access_proxy<F>(std::forward<accessor cv ref>(*this)), std::forward<Args>(args)...);
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
