# `basic_facade_builder::support_view`

```cpp
using support_view = basic_facade_builder</* see below */>;
```

The member type `support_view` of `basic_facade_builder<Cs, Rs, C>` adds necessary convention types to allow implicit conversion from [`proxy`](../proxy.md)`<F>` to [`proxy_view`](../proxy_view.md)`<F>` where `F` is a [facade](../facade.md) type built from `basic_facade_builder`.

Let `p` be a value of type `proxy<F>`, `ptr` be the contained value of `p` (if any), the conversion from type `proxy<F>&` to type `proxy_view<F>` is equivalent to `return raw-ptr{std::addressof(*ptr)}` if `p` contains a value, or otherwise equivalent to `return nullptr`. `observer-ptr` is an exposition-only type that `*observer-ptr`, `*std::as_const(observer-ptr)`, `*std::move(observer-ptr)` and `*std::move(std::as_const(observer-ptr))` are equivalent to `*ptr`, `*std::as_const(ptr)`, `*std::move(ptr)` and `*std::move(std::as_const(ptr))`, respectively.

## Notes

`support_view` is useful when a certain context does not take ownership of a `proxy` object. Similar to [`std::unique_ptr::get`](https://en.cppreference.com/w/cpp/memory/unique_ptr/get), [`std::shared_ptr::get`](https://en.cppreference.com/w/cpp/memory/shared_ptr/get) and the [borrowing mechanism in Rust](https://doc.rust-lang.org/rust-by-example/scope/borrow.html).

## Example

```cpp
#include <iostream>

#include "proxy.h"

struct RttiAware : pro::facade_builder
    ::support_rtti
    ::support_view
    ::build {};

int main() {
  pro::proxy<RttiAware> p = pro::make_proxy<RttiAware>(123);
  pro::proxy_view<RttiAware> pv = p;
  proxy_cast<int&>(*pv) = 456;  // Modifies the contained object of p
  std::cout << proxy_cast<int>(*pv) << "\n";  // Prints "456"
  std::cout << proxy_cast<int>(*p) << "\n";  // Prints "456"
}
```

## See Also

- [`add_convention`](add_convention.md)
