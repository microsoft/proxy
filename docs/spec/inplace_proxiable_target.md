# Concept `inplace_proxiable_target`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v4`

```cpp
template <class T, class F>
concept inplace_proxiable_target = proxiable<inplace-ptr<T>, F>;
```

See [`make_proxy_inplace`](make_proxy_inplace.md) for the definition of the exposition-only class template *inplace-ptr*.

## Example

```cpp
#include <array>

#include <proxy/proxy.h>

// By default, the maximum pointer size defined by pro::facade_builder
// is 2 * sizeof(void*). This value can be overridden by `restrict_layout`.
struct Any : pro::facade_builder::build {};

int main() {
  // sizeof(int) is usually not greater than sizeof(void*) for modern
  // 32/64-bit compilers
  static_assert(pro::inplace_proxiable_target<int, Any>);

  // sizeof(std::array<int, 100>) is usually greater than 2 * sizeof(void*)
  static_assert(!pro::inplace_proxiable_target<std::array<int, 100>, Any>);
}
```

## See Also

- [concept `proxiable`](proxiable.md)
- [concept `proxiable_target`](proxiable_target.md)
