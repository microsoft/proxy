# Macro `PRO_DEF_MEM_DISPATCH`

> Header: `proxy_macros.h` and `proxy.h`

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

`(1)` Equivalent to `PRO_DEF_MEM_DISPATCH(dispatch_name, func_name, func_name)`.

`(2)` Defines a class named `dispatch_name` of member function call expressions of `func_name` with accessibility via member function overloads named `accessibility_func_name`. Effectively equivalent to:

```cpp
struct dispatch_name {
  template <class T, class... Args>
  decltype(auto) operator()(T&& self, Args&&... args) const
      noexcept(noexcept(std::forward<T>(self).func_name(std::forward<Args>(args)...)))
      requires(requires { std::forward<T>(self).func_name(std::forward<Args>(args)...); }) {
    return std::forward<T>(self).func_name(std::forward<Args>(args)...);
  }

  template <class P, class D, class... Os>
  struct accessor {
    accessor() = delete;
  };
  template <class P, class D, class... Os>
      requires(sizeof...(Os) > 1u && (std::is_constructible_v<accessor<P, D, Os>> && ...))
  struct accessor<P, D, Os...> : accessor<P, D, Os>... {
    using accessor<P, D, Os>::accessibility_func_name ...;
  };
  template <class P, class D, class R, class... Args>
  struct accessor<P, D, R(Args...) cv ref noex> {
    R accessibility_func_name(Args... args) cv ref noex {
      return pro::proxy_invoke<D, R(Args...) cv ref noex>(static_cast<P cv <ref ? ref : &>>(*this), std::forward<Args>(args)...);
    }
  };
}
```

When headers from different major versions of the Proxy library can appear in the same translation unit (for example, Proxy 3 and Proxy 4), use the major-qualified form `PRO<major>_DEF_MEM_DISPATCH` (e.g., `PRO4_DEF_MEM_DISPATCH`).

## Example

```cpp
#include <iostream>
#include <string>
#include <vector>

#include <proxy/proxy.h>

PRO_DEF_MEM_DISPATCH(MemAt, at);

struct Dictionary : pro::facade_builder                                   //
                    ::add_convention<MemAt, std::string(int index) const> //
                    ::build {};

int main() {
  std::vector<const char*> v{"hello", "world"};
  pro::proxy<Dictionary> p = &v;
  std::cout << p->at(1) << "\n"; // Prints "world"
}
```

## See Also

- [macro `PRO_DEF_FREE_DISPATCH`](PRO_DEF_FREE_DISPATCH.md)
- [alias template `basic_facade_builder::add_convention`](basic_facade_builder/add_convention.md)
