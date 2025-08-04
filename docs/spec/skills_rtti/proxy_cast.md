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

Performs type-safe access to the contained object of `proxy<F>` where `F` is a [facade](../facade.md) type built from skills `rtti`, `indirect_rtti` or `direct_rtti`.

- `(1-3)` Let `p` be the contained value of the `proxy` object associated to `operand`. Returns `static_cast<T>(expr)` if `std::is_same_v<std::decay_t<decltype(expr)>, std::decay_t<T>>` is `true`, or otherwise, throws [`bad_proxy_cast`](../bad_proxy_cast.md). Specifically, `expr` is defined as
  - `(1)`: `*std::as_const(p)`.
  - `(2)`: `*p`.
  - `(3)`: `*std::move(p)`.
- `(4-6)` Let `p` be the contained value of `operand`. Returns `static_cast<T>(expr)` if `std::is_same_v<std::decay_t<p>, std::decay_t<T>>` is `true`, or otherwise, throws [`bad_proxy_cast`](../bad_proxy_cast.md). The behavior is undefined if `operand` does not contain a value. Specifically, `expr` is defined as
  - `(6)`: `std::as_const(p)`.
  - `(7)`: `p`.
  - `(8)`: `std::move(p)`.
- `(7-10)` Returns `std::addressof(proxy_cast<T>(*operand))` if the evaluation won't throw, or otherwise, returns `nullptr`.

These functions are not visible to ordinary [unqualified](https://en.cppreference.com/w/cpp/language/unqualified_lookup) or [qualified lookup](https://en.cppreference.com/w/cpp/language/qualified_lookup). It can only be found by [argument-dependent lookup](https://en.cppreference.com/w/cpp/language/adl) when `proxy_indirect_accessor<F>` (for `rtti` or `indirect_rtti`) or `proxy<F>` (for `direct_rtti`) is an associated class of the arguments. Usage of these functions is similar to [`std::any_cast`](https://en.cppreference.com/w/cpp/utility/any/any_cast).

## Example

```cpp
#include <iostream>

#include <proxy/proxy.h>

struct RttiAware : pro::facade_builder            //
                   ::add_skill<pro::skills::rtti> //
                   ::build {};

int main() {
  int v = 123;
  pro::proxy<RttiAware> p = &v;
  std::cout << proxy_cast<int>(*p) << "\n"; // Prints "123"
  proxy_cast<int&>(*p) = 456;
  std::cout << v << "\n"; // Prints "456"
  try {
    proxy_cast<double>(*p); // Throws
  } catch (const pro::bad_proxy_cast& e) {
    std::cout << e.what() << "\n"; // Prints an explanatory string
  }
  int* ptr1 = proxy_cast<int>(&*p);
  std::cout << ptr1 << "\n"; // Prints an address
  std::cout << &v << "\n";   // Prints the same address as above
  double* ptr2 = proxy_cast<double>(&*p);
  std::cout << ptr2 << "\n"; // Prints "0"
}
```

## See Also

- [function `proxy_typeid`](proxy_typeid.md)
