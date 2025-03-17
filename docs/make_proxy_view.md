# Function template `make_proxy_view`

The definition of `make_proxy_view` makes use of an exposition-only class template *observer-ptr*. `observer-ptr<T>` contains a raw pointer to an object of type `T`, and provides `operator*` for access with the same qualifiers.

```cpp
template <facade F, class T>
proxy_view<F> make_proxy_view(T& value) noexcept;
```

Creates a `proxy_view<F>` object containing a value `p` of type `observer-ptr<T>`, where `*p` is direct-non-list-initialized with `&value`. If [`proxiable_target<T, F>`](proxiable_target.md) is `false`, the program is ill-formed and diagnostic messages are generated.

## Return Value

The constructed `proxy_view` object.

## Example

```cpp
#include <iostream>
#include <map>
#include <string>

#include "proxy.h"

PRO_DEF_MEM_DISPATCH(MemAt, at);

struct ResourceDictionary : pro::facade_builder
    ::add_convention<MemAt, std::string&(int index), const std::string&(int index) const>
    ::build {};

int main() {
  std::map<int, std::string> dict;
  dict[1] = "init";
  pro::proxy_view<ResourceDictionary> pv = pro::make_proxy_view<ResourceDictionary>(dict);
  static_assert(std::is_same_v<decltype(pv->at(1)), std::string&>, "Non-const overload");
  static_assert(std::is_same_v<decltype(std::as_const(pv)->at(1)), const std::string&>, "Const overload");
  std::cout << std::as_const(pv)->at(1) << "\n";  // Invokes the const overload and prints "init"
  pv->at(1) = "modified";  // Invokes the non-const overload
  std::cout << std::as_const(pv)->at(1) << "\n";  // Invokes the const overload and prints "modified"
}
```

## See Also

- [concept `proxiable_target`](proxiable_target.md)
- [`basic_facade_builder::support_view`](basic_facade_builder/support_view.md)