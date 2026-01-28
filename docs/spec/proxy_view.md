# Alias template `proxy_view`<br />Class template `observer_facade`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v4`  
> Since: 3.2.0

```cpp
template <facade F>
struct observer_facade;

template <facade F>
using proxy_view = proxy<observer_facade<F>>;
```

`proxy_view<F>` is a non-owning, trivially copyable, trivially relocatable view of an object that models [`proxiable_target<T, F>`](proxiable_target.md). It behaves like a `proxy<F>` except that it never owns the lifetime of the underlying object.

`observer_facade<F>` adapts an existing [facade](facade.md) `F` for this non-owning use. The adaptation preserves only those parts of `F` that remain semantically valid when the storage is reduced to a single pointer and modifies substitution conversions so that view-ness is preserved (substitution that would have produced an owning `proxy<G>` instead produces a `proxy_view<G>`).

## Member Types of `observer_facade`

| Name               | Description                                                  |
| ------------------ | ------------------------------------------------------------ |
| `convention_types` | A [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type transformed from `typename F::convention_types`. Specifically, for each convention `C` in `typename F::convention_types`:<br/> - If `C::is_direct` is `false`, include `C` unchanged.<br/> - Otherwise, if `typename C::dispatch_type` is [`substitution_dispatch`](./substitution_dispatch/README.md), include a transformed convention `C'` whose<br/>  * `is_direct` is `true` and `dispatch_type` is still `substitution_dispatch`.<br/>  * For every overload `O` in `typename C::overload_types` with signature (after cv/ref/noexcept qualifiers) returning a `proxy<G>`, replace its return type with `proxy_view<G>` while preserving qualifiers and `noexcept`.<br/> - Otherwise `C` is discarded. |
| `reflection_types` | A [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type transformed from `typename F::reflection_types`. Specifically, for each reflection type `R` in `typename F::reflection_types`, `R` is included when `R::is_direct` is `false`, or otherwise discarded. |

## Member Constants of `observer_facade`

| Name                                   | Definition                  |
| -------------------------------------- | --------------------------- |
| `max_size` [static] [constexpr]        | `sizeof(void*)`             |
| `max_align` [static] [constexpr]       | `alignof(void*)`            |
| `copyability` [static] [constexpr]     | `constraint_level::trivial` |
| `relocatability` [static] [constexpr]  | `constraint_level::trivial` |
| `destructibility` [static] [constexpr] | `constraint_level::trivial` |

## Example

```cpp
#include <cstdio>
#include <map>

#include <proxy/proxy.h>

template <class K, class V>
struct FMap
    : pro::facade_builder                                              //
      ::add_convention<pro::operator_dispatch<"[]">, V&(const K& key)> //
      ::build {};

int main() {
  std::map<int, int> v;
  pro::proxy_view<FMap<int, int>> p = &v;
  (*p)[1] = 123;
  printf("%d\n", v.at(1)); // Prints "123"
}
```

## See Also

- [alias template `skills::as_view`](skills_as_view.md)
- [function template `make_proxy_view`](make_proxy_view.md)
