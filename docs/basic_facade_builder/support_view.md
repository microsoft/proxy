# `basic_facade_builder::add_view`

```cpp
template <class F>
using add_view = basic_facade_builder</* see below */>;
```

The alias template `add_view` of `basic_facade_builder<Cs, Rs, C>` adds necessary convention types to allow implicit conversion from [`proxy`](../proxy.md)`<F1>` to [`proxy_view`](../observer_facade.md)`<F>` where `F1` is a [facade](../facade.md) type built from `basic_facade_builder`.

Let `p` be a value of type `proxy<F>`, `ptr` be the contained value of `p` (if any).

- When `F` is not `const`-qualified, the conversion from type `proxy<F1>&` to type `proxy_view<F>` is equivalent to `return std::addressof(*ptr)` if `p` contains a value, or otherwise equivalent to `return nullptr`.
- When `F` is `const`-qualified, the conversion from type `const proxy<F1>&` to type `proxy_view<F>` is equivalent to `return std::addressof(*std::as_const(ptr))` if `p` contains a value, or otherwise equivalent to `return nullptr`.

## Notes

`add_view` is useful when a certain context does not take ownership of a `proxy` object. Similar to [`std::unique_ptr::get`](https://en.cppreference.com/w/cpp/memory/unique_ptr/get), [`std::shared_ptr::get`](https://en.cppreference.com/w/cpp/memory/shared_ptr/get) and the [borrowing mechanism in Rust](https://doc.rust-lang.org/rust-by-example/scope/borrow.html).

## Example

```cpp
#include <iomanip>
#include <iostream>
#include <string>

#include "proxy.h"

struct RttiAware : pro::facade_builder
    ::support_rtti
    ::add_view<RttiAware>
    ::add_view<const RttiAware>
    ::build {};

int main() {
  pro::proxy<RttiAware> p = pro::make_proxy<RttiAware>(123);
  pro::proxy_view<RttiAware> pv = p;
  pro::proxy_view<const RttiAware> pcv = p;
  proxy_cast<int&>(*pv) = 456;  // Modifies the contained object of p
  std::cout << proxy_cast<const int&>(*pcv) << "\n";  // Prints: "456"
}
```

## See Also

- [`add_convention`](add_convention.md)
