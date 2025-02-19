# Alias template `proxy_view`<br />Class template `observer_facade`

```cpp
template <class F>
struct observer_facade;  // since 3.2.0

template <class F>
using proxy_view = proxy<observer_facade<F>>;  // since 3.2.0
```

Class template `observer_facade` is a [facade](facade.md) type for observer pointers (e.g., raw pointers) potentially dereferenced from a `proxy` object. To instantiate `observer_facade<F>`, `F` shall model [concept `facade`](facade.md).

## Member Types of `observer_facade`

| Name               | Description                                                  |
| ------------------ | ------------------------------------------------------------ |
| `convention_types` | A [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type transformed from `typename F::convention_types`. Specifically, for each convention type `C` defined in `typename F::convention_types`, `C` is reserved when `C::is_direct` is `false` or when `C` is an upward conversion convention added via [`basic_facade_builder::add_facade<?, true>`](basic_facade_builder/add_facade.md), or otherwise filtered out. |
| `reflection_types` | A [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type transformed from `typename F::reflection_types`. Specifically, for each reflection type `R` in `typename F::reflection_types`, `R` is reserved when `R::is_direct` is `false`, or otherwise filtered out. |

## Member Constants of `observer_facade`

| Name                               | Description                                                  |
| ---------------------------------- | ------------------------------------------------------------ |
| `constraints` [static] [constexpr] | A value of type `proxiable_ptr_constraints` indicating constraints for an observer pointer type. |

## Example

```cpp
#include <map>

#include "proxy.h"

template <class K, class V>
struct FMap : pro::facade_builder
    ::add_convention<pro::operator_dispatch<"[]">, V&(const K& key)>
    ::build {};

int main() {
  std::map<int, int> v;
  pro::proxy_view<FMap<int, int>> p = &v;
  (*p)[1] = 123;
  printf("%d\n", v.at(1));  // Prints "123"
}
```

## See Also

[`basic_facade_builder::support_view`](basic_facade_builder/support_view.md)
