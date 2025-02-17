# Class template `facade_aware_overload_t`

```cpp
template <template <class> class O>
struct facade_aware_overload_t { facade_aware_overload_t() = delete; };
```

Class template `facade_aware_overload_t<O>` specifies a facade-aware overload template `O`. It is useful when modeling an [overload](ProOverload.md) type of a [facade](facade.md) type that recursively depends on the facade type itself.

## Notes

`facade_aware_overload_t` can be used to define a convention in a base facade type, and is portable to the definition of another facade type via [`basic_facade_builder::add_facade`](basic_facade_builder/add_facade.md). It can also effectively avoid a facade type being implicitly instantiated when it is incomplete.

## Example

```cpp
#include <iostream>

#include "proxy.h"

template <class F>
using BinaryOverload = pro::proxy<F>(const pro::proxy_indirect_accessor<F>& rhs) const;

template <class T, class F>
pro::proxy<F> operator+(const T& value, const pro::proxy_indirect_accessor<F>& rhs)
    requires(!std::is_same_v<T, pro::proxy_indirect_accessor<F>>)
    { return pro::make_proxy<F, T>(value + proxy_cast<const T&>(rhs)); }

template <class T, class F>
pro::proxy<F> operator*(const T& value, const pro::proxy_indirect_accessor<F>& rhs)
    requires(!std::is_same_v<T, pro::proxy_indirect_accessor<F>>)
    { return pro::make_proxy<F, T>(value * proxy_cast<const T&>(rhs)); }

struct Addable : pro::facade_builder
    ::support_rtti
    ::add_convention<pro::operator_dispatch<"+">, pro::facade_aware_overload_t<BinaryOverload>>
    ::build {};

struct Multipliable : pro::facade_builder
    ::support_rtti
    ::add_convention<pro::operator_dispatch<"*">, pro::facade_aware_overload_t<BinaryOverload>>
    ::build {};

struct BasicNumber : pro::facade_builder
    ::support_format
    ::add_facade<Addable>
    ::add_facade<Multipliable>
    ::build {};

int main() {
  pro::proxy<BasicNumber> p1 = pro::make_proxy<BasicNumber>(1);
  pro::proxy<BasicNumber> p2 = pro::make_proxy<BasicNumber>(2);
  pro::proxy<BasicNumber> p3 = *p1 + *p2;  // p3 is int(3)
  p3 = *p3 * *p2;  // p3 becomes int(6)
  std::cout << std::format("{}\n", *p3);  // Prints "6"
}
```

## See Also

- [*ProOverload* requirements](ProOverload.md)
- [`basic_facade_builder::support_view`](basic_facade_builder/support_view.md)
