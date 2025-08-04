# Alias template `as_weak`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v4::skills`  
> Since: 4.0.0

```cpp
template <class FB>
using as_weak = /* see below */;
```

The alias template `as_weak` modifies a specialization of [`basic_facade_builder`](basic_facade_builder/README.md) to allow implicit conversion from [`proxy`](proxy/README.md)`<F>` to [`weak_proxy`](weak_proxy.md)`<F>`, where `F` is a built [facade](facade.md) type.

Let `p` be a value of type `proxy<F>`, `ptr` be the contained value of `p` (if any), `Ptr` be the type of `ptr`, the conversion from type `const proxy<F>&` to type `weak_proxy<F>` is equivalent to `return typename Ptr::weak_type{p}` if `p` contains a value, or otherwise equivalent to `return nullptr`.

## Example

```cpp
#include <iostream>

#include <proxy/proxy.h>

struct RttiAware : pro::facade_builder               //
                   ::add_skill<pro::skills::rtti>    //
                   ::add_skill<pro::skills::as_weak> //
                   ::build {};

int main() {
  pro::proxy<RttiAware> p1 = pro::make_proxy_shared<RttiAware>(123);
  pro::weak_proxy<RttiAware> p2 = p1;
  proxy_cast<int&>(*p1) = 456; // Modifies the contained object of p1

  pro::proxy<RttiAware> p3 = p2.lock();
  p1.reset();
  std::cout << proxy_cast<int>(*p3) << "\n"; // Prints "456"

  p3.reset();
  pro::proxy<RttiAware> p4 = p2.lock();
  std::cout << std::boolalpha << p4.has_value() << "\n"; // Prints "false"
}
```

## See Also

- [`basic_facade_builder::add_skill`](basic_facade_builder/add_skill.md)
- [class template `facade_aware_overload_t`](facade_aware_overload_t.md)
