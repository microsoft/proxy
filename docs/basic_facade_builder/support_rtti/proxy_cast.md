# Function template `proxy_cast`

```cpp
// (1)
template <class T>
T proxy_cast(const proxy_indirect_accessor<F>& operand);

// (2)
template <class T>
T proxy_cast(proxy_indirect_accessor<F>& operand);

// (3)
template <class T>
T proxy_cast(proxy_indirect_accessor<F>&& operand);

// (4)
template <class T>
T proxy_cast(const proxy<F>& operand);

// (5)
template <class T>
T proxy_cast(proxy<F>& operand);

// (6)
template <class T>
T proxy_cast(proxy<F>&& operand);

// (7)
template <class T>
const T* proxy_cast(const proxy_indirect_accessor<F>* operand) noexcept;

// (8)
template <class T>
T* proxy_cast(proxy_indirect_accessor<F>* operand) noexcept;

// (9)
template <class T>
const T* proxy_cast(const proxy<F>* operand) noexcept;

// (10)
template <class T>
T* proxy_cast(proxy<F>* operand) noexcept;
```

Performs type-safe access to the contained object of `proxy<F>` where `F` is a [facade](../../facade.md) type built from `basic_facade_builder`.

- `(1-3)` Let `p` be [`access_proxy`](../../access_proxy.md)`<F>(operand)`, `ptr` be the contained value of `p` (if any). If `p` does not contain a value or `std::is_same_v<std::decay_t<decltype(expr)>, std::decay_t<T>>` is `false`, throws [`bad_proxy_cast`](../../bad_proxy_cast.md). Otherwise, returns `static_cast<T>(expr)`. Specifically, `expr` is defined as
  - `(1)`: `*std::as_const(ptr)`.
  - `(2)`: `*ptr`.
  - `(3)`: `*std::move(ptr)`.
- `(4-6)` Let `ptr` be the contained value of `operand` (if any). If `operand` does not contain a value or `std::is_same_v<std::decay_t<ptr>, std::decay_t<T>>` is `false`, throws [`bad_proxy_cast`](../../bad_proxy_cast.md). Otherwise, returns `static_cast<T>(expr)`. Specifically, `expr` is defined as
  - `(6)`: `std::as_const(ptr)`.
  - `(7)`: `ptr`.
  - `(8)`: `std::move(ptr)`.
- `(7-10)` Returns `&proxy_cast<T>(*operand)` if the evaluation won't throw, or otherwise, returns `nullptr`.

These functions are not visible to ordinary [unqualified](https://en.cppreference.com/w/cpp/language/unqualified_lookup) or [qualified lookup](https://en.cppreference.com/w/cpp/language/qualified_lookup). It can only be found by [argument-dependent lookup](https://en.cppreference.com/w/cpp/language/adl) when `proxy_indirect_accessor<F>` (if `support_rtti` or `support_indirect_rtti` is specified) or `proxy<F>` (if `support_direct_rtti` is specified) is an associated class of the arguments. Usage of these functions is similar to [`std::any_cast`](https://en.cppreference.com/w/cpp/utility/any/any_cast).

## Example

```cpp
#include <iostream>

#include "proxy.h"

struct RttiAware : pro::facade_builder
    ::support_rtti
    ::build {};

int main() {
  int v = 123;
  pro::proxy<RttiAware> p;
  try {
    proxy_cast<int>(*p);  // Throws
  } catch (const pro::bad_proxy_cast& e) {
    std::cout << e.what() << "\n";  // Prints an explanatory string
  }
  p = &v;
  std::cout << proxy_cast<int>(*p) << "\n";  // Prints: "123"
  proxy_cast<int&>(*p) = 456;
  std::cout << v << "\n";  // Prints: "456"
  try {
    proxy_cast<double>(*p);  // Throws
  } catch (const pro::bad_proxy_cast& e) {
    std::cout << e.what() << "\n";  // Prints an explanatory string
  }
  int* ptr1 = proxy_cast<int>(&*p);
  std::cout << std::boolalpha << ptr1 << "\n";  // Prints an address
  std::cout << std::boolalpha << &v << "\n";  // Prints the same address as above
  double* ptr2 = proxy_cast<double>(&*p);
  std::cout << ptr2 << "\n";  // Prints "0"
}
```

## See Also

- [function `proxy_typeid`](proxy_typeid.md)
