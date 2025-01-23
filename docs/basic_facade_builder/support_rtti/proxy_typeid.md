# Function `proxy_typeid`

```cpp
// (1)
const std::type_info& proxy_typeid(const proxy_indirect_accessor<F>& operand) noexcept;

// (2)
const std::type_info& proxy_typeid(const proxy<F>& operand) noexcept;
```

Returns the `typeid` of the contained type of `proxy<F>` where `F` is a [facade](../../facade.md) type built from `basic_facade_builder`.

- `(1)` Let `p` be [`access_proxy`](../../access_proxy.md)`<F>(operand)`, `ptr` be the contained value of `p` (if any). Returns `typeid(std::decay_t<decltype(*std::as_const(ptr))>)` if `p` contains a value, or otherwise, `typeid(void)`.
- `(2)` Let `ptr` be the contained value of `operand` (if any). Returns `typeid(std::decay_t<decltype(ptr)>)` if `operand` contains a value `ptr`, or otherwise, `typeid(void)`.

These functions are not visible to ordinary [unqualified](https://en.cppreference.com/w/cpp/language/unqualified_lookup) or [qualified lookup](https://en.cppreference.com/w/cpp/language/qualified_lookup). It can only be found by [argument-dependent lookup](https://en.cppreference.com/w/cpp/language/adl) when `proxy_indirect_accessor<F>` (if `support_rtti` or `support_indirect_rtti` is specified) or `proxy<F>` (if `support_direct_rtti` is specified) is an associated class of the arguments.

## Example

```cpp
#include <iostream>

#include "proxy.h"

struct RttiAware : pro::facade_builder
    ::support_rtti
    ::build {};

int main() {
  pro::proxy<RttiAware> p;
  std::cout << proxy_typeid(*p).name() << "\n";  // Prints: "v" (assuming GCC)
  p = pro::make_proxy<RttiAware>(123);
  std::cout << proxy_typeid(*p).name() << "\n";  // Prints: "i" (assuming GCC)
}
```

## See Also

- [function template `proxy_cast`](proxy_cast.md)
