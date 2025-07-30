# Function `proxy_typeid`

```cpp
// (1)
const std::type_info& proxy_typeid(const proxy_indirect_accessor<F>& operand) noexcept;

// (2)
const std::type_info& proxy_typeid(const proxy<F>& operand) noexcept;
```

Returns the `typeid` of the contained type of `operand`.

- `(1)` Let `P` be the contained type of the `proxy` object associated to `operand`. Returns `typeid(typename std::pointer_traits<P>::element_type)`.
- `(2)` Let `P` be the contained type of `operand`. Returns `typeid(P)`. The behavior is undefined if `operand` does not contain a value.

These functions are not visible to ordinary [unqualified](https://en.cppreference.com/w/cpp/language/unqualified_lookup) or [qualified lookup](https://en.cppreference.com/w/cpp/language/qualified_lookup). It can only be found by [argument-dependent lookup](https://en.cppreference.com/w/cpp/language/adl) when `proxy_indirect_accessor<F>` (for `rtti` or `indirect_rtti`) or `proxy<F>` (for `direct_rtti`) is an associated class of the arguments.

## Example

```cpp
#include <iostream>

#include <proxy/proxy.h>

struct RttiAware : pro::facade_builder            //
                   ::add_skill<pro::skills::rtti> //
                   ::build {};

int main() {
  pro::proxy<RttiAware> p = pro::make_proxy<RttiAware>(123);
  std::cout << proxy_typeid(*p).name() << "\n"; // Prints "i" (assuming GCC)
}
```

## See Also

- [function template `proxy_cast`](proxy_cast.md)
