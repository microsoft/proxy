# Function template `make_proxy_inplace`

The definition of `make_proxy_inplace` makes use of an exposition-only class template *sbo-ptr*. Similar to [`std::optional`](https://en.cppreference.com/w/cpp/utility/optional), `sbo-ptr<T>` contains the storage for an object of type `T`, manages its lifetime, and provides `operator*` for access with the same qualifiers. However, it does not necessarily support the state where the contained object is absent. `sbo-ptr<T>` has the same size and alignment as `T`.

```cpp
// (1)
template <facade F, inplace_proxiable_target<F> T, class... Args>
proxy<F> make_proxy_inplace(Args&&... args)
    noexcept(std::is_nothrow_constructible_v<T, Args...>);

// (2)
template <facade F, inplace_proxiable_target<F> T, class U, class... Args>
proxy<F> make_proxy_inplace(std::initializer_list<U> il, Args&&... args)
    noexcept(std::is_nothrow_constructible_v<
        T, std::initializer_list<U>&, Args...>);

// (3)
template <facade F, class T>
proxy<F> make_proxy_inplace(T&& value)
    noexcept(std::is_nothrow_constructible_v<std::decay_t<T>, T>)
    requires(inplace_proxiable_target<std::decay_t<T>, F>);
```

`(1)` Creates a `proxy<F>` object containing a value `p` of type `sbo-ptr<T>`, where `*p` is direct-non-list-initialized with `std::forward<Args>(args)...`.

`(2)` Creates a `proxy<F>` object containing a value `p` of type `sbo-ptr<T>`, where `*p` is direct-non-list-initialized with `il, std::forward<Args>(args)...`.

`(3)` Creates a `proxy<F>` object containing a value `p` of type `sbo-ptr<std::decay_t<T>>`, where `*p` is direct-non-list-initialized with `std::forward<T>(value)`.

## Return Value

The constructed `proxy` object.

## Exceptions

Throws any exception thrown by the constructor of `T`.

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
  pro::proxy<Any> p1 = pro::make_proxy_inplace<Any>(123);

  // sizeof(std::array<int, 100>) is usually greater than 2 * sizeof(void*)
  // pro::proxy<Any> p2 = pro::make_proxy_inplace<Any, std::array<int, 100>>();  // Won't compile
}
```

## See Also

- [concept `inplace_proxiable_target`](inplace_proxiable_target.md)
- [function template `make_proxy`](make_proxy.md)
