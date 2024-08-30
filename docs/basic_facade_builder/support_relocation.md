# `basic_facade_builder::support_relocation`

```cpp
template <constraint_level CL>
using support_relocation = basic_facade_builder</* see below */>;
```

The alias template `support_relocation` of `basic_facade_builder<Cs, Rs, C>` adds relocatability support to the template parameters, specifically `C::relocatability`. After the operation, `C::relocatability` becomes `std::max(C::relocatability, CL)`.

## Notes

If no relocatability support is applied before specifying [`build`](build.md), the default value of `build::constraints.relocatability` is `pro::constraint_level::nothrow`.

## Example

```cpp
#include <memory>

#include "proxy.h"

struct Movable : pro::facade_builder::build {};

struct Trivial : pro::facade_builder
    ::support_copy<pro::constraint_level::trivial>
    ::support_relocation<pro::constraint_level::trivial>
    ::support_destruction<pro::constraint_level::trivial>
    ::build {};

int main() {
  pro::proxy<Movable> p1 = std::make_unique<int>(123);
  // pro::proxy<Trivial> p2 = std::make_unique<int>(456);  // Won't compile
  double v = 3.14;
  pro::proxy<Trivial> p3 = &v;  // Compiles because double* is trivial
}
```

## See Also

- [`support_copy`](support_copy.md)
