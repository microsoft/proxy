# Class template `observer_facade`<br />Alias template `proxy_view`

```cpp
template <class F>
struct observer_facade;

template <class F>
using proxy_view = proxy<observer_facade<F>>;
```

Class template `observer_facade` is a [facade](facade.md) type for raw pointers potentially dereferenced from a `proxy` object. To instantiate `observer_facade<F>`, `F` shall model [concept `facade`](facade.md) and optionally `const`-qualified.

## Member Types of `observer_facade`

| Name               | Description                                                  |
| ------------------ | ------------------------------------------------------------ |
| `convention_types` | A [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type transformed from `typename F::convention_types`. Specifically, 1) For each convention type `C` defined in `typename F::convention_types`, `C` is reserved when `C::is_direct` is `false`, or otherwise filtered out. 2) For each overload type `O` defined in `typename C::overload_types` of each type `C`, `O` is reserved and transformed into the `const`-qualified type (e.g., `void(int)` is transformed into `void(int) const`) when `O` is not rvalue-reference-qualified and matches the `const`-ness of `F` except when `C` is an upward conversion convention added via [`basic_facade_builder::add_facade<?, true>`](basic_facade_builder/add_facade.md), or otherwise filtered out. 3) Each type `C` is reserved when `std::tuple_size_v<typename C::overload_types>` is greater than `0`, or otherwise filtered out. |
| `reflection_types` | A [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type transformed from `typename F::reflection_types`. Specifically, for each reflection type `R` in `typename F::reflection_types`, `R` is reserved when `R::is_direct` is `false`, or otherwise filtered out. |

## Member Constants of `observer_facade`

| Name                               | Description                                                  |
| ---------------------------------- | ------------------------------------------------------------ |
| `constraints` [static] [constexpr] | A value of type `proxiable_ptr_constraints` indicating constraints for a raw pointer type. |

## Example

```cpp
#include <map>

#include "proxy.h"

PRO_DEF_MEM_DISPATCH(MemAt, at);

template <class K, class V>
struct FMap : pro::facade_builder
    ::add_convention<MemAt, V&(const K& key), const V&(const K& key) const>
    ::build {};

int main() {
  std::map<int, int> v{{1, 2}};

  pro::proxy_view<FMap<int, int>> p1 = &v;
  static_assert(std::is_same_v<decltype(p1->at(1)), int&>);
  p1->at(1) = 3;
  printf("%d\n", v.at(1));  // Prints "3"

  pro::proxy_view<const FMap<int, int>> p2 = &std::as_const(v);
  static_assert(std::is_same_v<decltype(p2->at(1)), const int&>);
  // p2->at(1) = 4; won't compile
  printf("%d\n", p2->at(1));  // Prints "3"
}
```

## See also

[`basic_facade_builder::support_view`](basic_facade_builder/support_view.md)
