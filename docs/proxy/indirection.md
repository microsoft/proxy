# `proxy::operator->`, `proxy::operator*`

The definitions of `proxy::operator->` and `proxy::operator*` make use of the following exposition-only constant and type alias:

```cpp
static constexpr bool has-indirection = see below;  // exposition only
using indirect-accessor = see below;  // exposition only
```

As per [`facade<F>`](../facade.md), `typename F::convention_types` shall be a [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type containing any number of distinct types `Cs`. Let `Cs2` be the types in `Cs` where each type `C` meets the [*ProAccessible* requirements](../ProAccessible.md) of `F` and `C::is_direct` is `false`. *has-indirection* is `true` if `Cs2` contains at least one type; otherwise, it is `false`. *indirect-accessor* is a non-copyable type that inherits from every type in `Cs2`.

```cpp
// (1)
indirect-accessor* operator->() noexcept requires(has-indirection);
const indirect-accessor* operator->() const noexcept requires(has-indirection);

// (2)
indirect-accessor& operator*() & noexcept requires(has-indirection);
const indirect-accessor& operator*() const& noexcept requires(has-indirection);
indirect-accessor&& operator*() && noexcept requires(has-indirection);
const indirect-accessor&& operator*() const&& noexcept requires(has-indirection);
```

These operators access the accessors of the indirect conventions, as if dereferencing the contained value.

- `(1)` Returns a pointer to the *indirect-accessor*.
- `(2)` Returns a reference to the *indirect-accessor*.

The behavior is undefined if `*this` does not contain a value.

## Notes

These operators do not check whether the `proxy` contains a value. To check whether the `proxy` contains a value, call [`has_value()`](operator_bool.md) or use [operator ==](friend_operator_equality.md).

## Example

```cpp
#include <iostream>
#include <string>
#include <vector>

#include "proxy.h"

PRO_DEF_MEM_DISPATCH(MemSize, size);

struct BasicContainer : pro::facade_builder
    ::add_convention<MemSize, std::size_t() const& noexcept>
    ::build {};

PRO_DEF_FREE_DISPATCH(FreeToString, std::to_string, ToString);

struct Stringable : pro::facade_builder
    ::add_convention<FreeToString, std::string()>
    ::build {};

int main() {
  std::vector<int> v(10);
  pro::proxy<BasicContainer> p0 = &v;
  std::cout << p0->size() << "\n";  // Prints "10"
  std::cout << (*p0).size() << "\n";  // Prints "10"

  pro::proxy<Stringable> p1 = pro::make_proxy<Stringable>(123);
  std::cout << ToString(*p1) << "\n";  // Prints "123"
}
```

## See Also

- [function template `access_proxy`](../access_proxy.md)
- [function template `proxy_invoke`](../proxy_invoke.md)
