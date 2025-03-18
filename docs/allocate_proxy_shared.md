# Function template `allocate_proxy_shared`

The definition of `allocate_proxy_shared` makes use of exposition-only class templates *strong-compact-ptr* and *weak-compact-ptr*. Their semantics are similar to [`std::shared_ptr`](https://en.cppreference.com/w/cpp/memory/shared_ptr) and [`std::weak_ptr`](https://en.cppreference.com/w/cpp/memory/weak_ptr), but do not provide a polymorphic deleter. Their size and alignment are guaranteed not to be greater than those of a raw pointer type. `strong-compact-ptr<T, Alloc>` is conditionally convertible to `weak-compact-ptr<T, Alloc>` only if necessary. Similar to [`std::optional`](https://en.cppreference.com/w/cpp/utility/optional), `strong-compact-ptr<T, Alloc>` provides `operator*` for accessing the managed object of type `T` with the same qualifiers.

```cpp
// (1)
template <facade F, class T, class Alloc, class... Args>
proxy<F> allocate_proxy_shared(const Alloc& alloc, Args&&... args);  // freestanding-deleted

// (2)
template <facade F, class T, class Alloc, class U, class... Args>
proxy<F> allocate_proxy_shared(const Alloc& alloc, std::initializer_list<U> il, Args&&... args);  // freestanding-deleted

// (3)
template <facade F, class Alloc, class T>
proxy<F> allocate_proxy_shared(const Alloc& alloc, T&& value);  // freestanding-deleted
```

`(1)` Creates a `proxy<F>` object containing a value `p` of type `strong-compact-ptr<T, Alloc>`, where `*p` is direct-non-list-initialized with `std::forward<Args>(args)...`.

`(2)` Creates a `proxy<F>` object containing a value `p` of type `strong-compact-ptr<T, Alloc>`, where `*p` is direct-non-list-initialized with `il, std::forward<Args>(args)...`.

`(3)` Creates a `proxy<F>` object containing a value `p` of type `strong-compact-ptr<std::decay_t<T>, Alloc>`, where `*p` is direct-non-list-initialized with `std::forward<T>(value)`.

For `(1-3)`, if [`proxiable_target<std::decay_t<T>, F>`](proxiable_target.md) is `false`, the program is ill-formed and diagnostic messages are generated.

## Return Value

The constructed `proxy` object.

## Exceptions

Throws any exception thrown by allocation or the constructor of `T`.

## Notes

The implementation of `strong-compact-ptr` may vary depending on the definition of `F`. Specifically, when `F` does not support weak ownership via [`basic_facade_builder::support_weak`](basic_facade_builder/support_weak.md), `strong-compact-ptr<T, Alloc>` is not convertible to `weak-compact-ptr<T, Alloc>`, which leaves more room for optimization.

## Example

```cpp
#include <iostream>
#include <memory_resource>

#include "proxy.h"

struct RttiAware : pro::facade_builder
    ::support_copy<pro::constraint_level::nothrow>
    ::support_rtti
    ::build {};

int main() {
  std::pmr::unsynchronized_pool_resource pool;
  std::pmr::polymorphic_allocator<> alloc{&pool};
  pro::proxy<RttiAware> p1 = pro::allocate_proxy_shared<RttiAware>(alloc, 1);
  pro::proxy<RttiAware> p2 = p1;
  proxy_cast<int&>(*p1) += 2;
  std::cout << proxy_cast<int>(*p2) << "\n";  // Prints "3"
}
```

## See Also

- [function template `allocate_proxy`](allocate_proxy.md)
- [named requirements *Allocator*](https://en.cppreference.com/w/cpp/named_req/Allocator)