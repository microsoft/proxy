# Function template `allocate_proxy`

The definition of `allocate_proxy` makes use of an exposition-only class template *allocated-ptr*. An object of type `allocated-ptr<T, Alloc>` allocates the storage for another object of type `T` with an allocator of type `Alloc` and manages the lifetime of this contained object. Similar to [`std::optional`](https://en.cppreference.com/w/cpp/utility/optional), `allocated-ptr<T, Alloc>` provides `operator*` for accessing the managed object of type `T` with the same qualifiers, but does not necessarily support the state where the contained object is absent.

```cpp
// (1)
template <facade F, class T, class Alloc, class... Args>
proxy<F> allocate_proxy(const Alloc& alloc, Args&&... args);  // freestanding-deleted

// (2)
template <facade F, class T, class Alloc, class U, class... Args>
proxy<F> allocate_proxy(const Alloc& alloc, std::initializer_list<U> il, Args&&... args);  // freestanding-deleted

// (3)
template <facade F, class Alloc, class T>
proxy<F> allocate_proxy(const Alloc& alloc, T&& value);  // freestanding-deleted
```

`(1)` Creates a `proxy<F>` object containing a value `p` of type `allocated-ptr<T, Alloc>`, where `*p` is direct-non-list-initialized with `std::forward<Args>(args)...`.

`(2)` Creates a `proxy<F>` object containing a value `p` of type `allocated-ptr<T, Alloc>`, where `*p` is direct-non-list-initialized with `il, std::forward<Args>(args)...`.

`(3)` Creates a `proxy<F>` object containing a value `p` of type `allocated-ptr<std::decay_t<T>, Alloc>`, where `*p` is direct-non-list-initialized with `std::forward<T>(value)`.

## Return Value

The constructed `proxy` object.

## Exceptions

Throws any exception thrown by allocation or the constructor of `T`.

## Notes

The implementation of `allocated-ptr` may vary depending on the definition of `F`. Specifically, when `F::constraints.max_size` and `F::constraints.max_align` are not large enough to hold both a pointer to the allocated memory and a copy of the allocator, `allocated-ptr` shall allocate additional storage for the allocator.

## Example

```cpp
#include <array>

#include "proxy.h"

// By default, the maximum pointer size defined by pro::facade_builder
// is 2 * sizeof(void*). This value can be overridden by `restrict_layout`.
struct Any : pro::facade_builder::build {};

int main() {
  // sizeof(std::array<int, 100>) is usually greater than 2 * sizeof(void*),
  // calling allocate_proxy has no limitation to the size and alignment of the target
  using Target = std::array<int, 100>;
  pro::proxy<Any> p1 = pro::allocate_proxy<Any, Target>(std::allocator<Target>{});
}
```

## See Also

- [function template `make_proxy`](make_proxy.md)
- [named requirements *Allocator*](https://en.cppreference.com/w/cpp/named_req/Allocator)
