# `basic_facade_builder::support_copy`

```cpp
template <constraint_level CL>
using support_copy = basic_facade_builder</* see below */>;
```

The alias template `support_copy` of `basic_facade_builder<Cs, Rs, C>` adds copyability support to the template parameters, specifically `C::copyability`. After the operation, `C::copyability` becomes `std::max(C::copyability, CL)`.

## Notes

If no copyability support is applied before specifying [`build`](build.md), the default value of `build::constraints.copyability` is `pro::constraint_level::none`.

## Example

```cpp
#include <memory>

#include "proxy.h"

struct Movable : pro::facade_builder::build {};

struct Copyable : pro::facade_builder
    ::support_copy<pro::constraint_level::nontrivial>
    ::build {};

int main() {
  pro::proxy<Movable> p1 = std::make_unique<int>(123);
  // pro::proxy<Copyable> p2 = std::make_unique<int>(123);  // Won't compile
  pro::proxy<Copyable> p3 = std::make_shared<int>(456);
  // auto p4 = p1;  // Won't compile
  auto p5 = p3;
}
```

## See Also

- [`support_relocation`](support_relocation.md)
