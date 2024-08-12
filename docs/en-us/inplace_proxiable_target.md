# Concept `inplace_proxiable_target`

```cpp
template <class T, class F>
concept inplace_proxiable_target = proxiable</* inplace-ptr<T> */, F>;
```

The concept `inplace_proxiable_target<T, F>` specifies that a value type `T`, when wrapped by an implementation-defined non-allocating pointer type, models a contained value type of [`proxy<F>`](proxy.md). The size and alignment of this implementation-defined pointer type are guaranteed to be equal to those of type `T`.

## Example

```cpp
#include <array>

#include "proxy.h"

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
- [function template `make_proxy_inplacce`](make_proxy_inplace.md)
