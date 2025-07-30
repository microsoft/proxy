# Alias template `as_view`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v4::skills`  
> Since: 4.0.0

```cpp
template <class FB>
using as_view = /* see below */;
```

The alias template `as_view` modifies a specialization of [`basic_facade_builder`](basic_facade_builder/README.md) to allow implicit conversion from [`proxy`](proxy/README.md)`<F>` to [`proxy_view`](proxy_view.md)`<F>`, where `F` is a built [facade](facade.md) type.

Let `p` be a value of type `proxy<F>`, `ptr` be the contained value of `p` (if any), the conversion from type `proxy<F>&` to type `proxy_view<F>` is equivalent to `return observer-ptr{std::addressof(*ptr)}` if `p` contains a value, or otherwise equivalent to `return nullptr`. `observer-ptr` is an exposition-only type that `*observer-ptr`, `*std::as_const(observer-ptr)`, `*std::move(observer-ptr)` and `*std::move(std::as_const(observer-ptr))` are equivalent to `*ptr`, `*std::as_const(ptr)`, `*std::move(ptr)` and `*std::move(std::as_const(ptr))`, respectively.

## Notes

`as_view` is useful when a certain context does not take ownership of a `proxy` object. Similar to [`std::unique_ptr::get`](https://en.cppreference.com/w/cpp/memory/unique_ptr/get), [`std::shared_ptr::get`](https://en.cppreference.com/w/cpp/memory/shared_ptr/get) and the [borrowing mechanism in Rust](https://doc.rust-lang.org/rust-by-example/scope/borrow.html).

## Example

```cpp
#include <iostream>

#include <proxy/proxy.h>

struct RttiAware : pro::facade_builder               //
                   ::add_skill<pro::skills::rtti>    //
                   ::add_skill<pro::skills::as_view> //
                   ::build {};

int main() {
  pro::proxy<RttiAware> p = pro::make_proxy<RttiAware>(123);
  pro::proxy_view<RttiAware> pv = p;
  proxy_cast<int&>(*pv) = 456; // Modifies the contained object of p
  std::cout << proxy_cast<int>(*pv) << "\n"; // Prints "456"
  std::cout << proxy_cast<int>(*p) << "\n";  // Prints "456"
}
```

## See Also

- [`basic_facade_builder::add_skill`](basic_facade_builder/add_skill.md)
- [class template `facade_aware_overload_t`](facade_aware_overload_t.md)
