# `basic_facade_builder::support_weak`

```cpp
using support_weak = basic_facade_builder</* see below */>;
```

The member type `support_weak` of `basic_facade_builder<Cs, Rs, C>` adds necessary convention types to allow implicit conversion from [`proxy`](../proxy.md)`<F>` to [`weak_proxy`](../weak_proxy.md)`<F>` where `F` is a [facade](../facade.md) type built from `basic_facade_builder`.

Let `p` be a value of type `proxy<F>`, `ptr` of type `P` be the contained value of `p` (if any), the conversion from type `const proxy<F>&` to type `weak_proxy<F>` is equivalent to `return typename P::weak_type{ptr}` if `p` contains a value, or otherwise equivalent to `return nullptr`.

## Notes

`support_weak` is compatible with [`std::weak_ptr`](https://en.cppreference.com/w/cpp/memory/weak_ptr), and may generate more efficient code when working with [`make_proxy_shared`](../make_proxy_shared.md) or [`allocate_proxy_shared`](../allocate_proxy_shared.md). It is also compatible with any custom shared/weak ownership implementations if `typename P::weak_type{ptr}` is well-formed.

## Example

```cpp
#include <iostream>

#include "proxy.h"

struct Formattable : pro::facade_builder
    ::support_format
    ::support_weak
    ::build {};

int main() {
  pro::proxy<Formattable> p1 = pro::make_proxy_shared<Formattable>(123);
  pro::weak_proxy<Formattable> wp = p1;
  pro::proxy<Formattable> p2 = wp.lock();
  std::cout << std::boolalpha << p2.has_value() << "\n";  // Prints "true"
  std::cout << std::format("{}\n", *p2);  // Prints "123"

  p1.reset();
  p2.reset();
  p2 = wp.lock();
  std::cout << p2.has_value() << "\n";  // Prints "false"
}
```

## See Also

- [`add_convention`](add_convention.md)
