# `basic_facade_builder::support_destruction`

```cpp
template <constraint_level CL>
using support_destruction = basic_facade_builder</* see below */>;
```

The alias template `support_destruction` of `basic_facade_builder<Cs, Rs, C>` adds destruction support to the template parameters, specifically `C::destructibility`. After the operation, `C::destructibility` becomes `std::max(C::destructibility, CL)`.

## Notes

If no destructibility support is applied before specifying [`build`](build.md), the default value of `build::constraints.destructibility` is `pro::constraint_level::nothrow`.

## Example

```cpp
#include <type_traits>

#include "proxy.h"

struct Movable : pro::facade_builder::build {};

struct NonriviallyDestructible : pro::facade_builder
    ::support_relocation<pro::constraint_level::nontrivial>
    ::support_destruction<pro::constraint_level::nontrivial>
    ::build {};

int main() {
  static_assert(std::is_nothrow_destructible_v<pro::proxy<Movable>>);
  static_assert(!std::is_nothrow_destructible_v<pro::proxy<NonriviallyDestructible>>);
}
```

## See Also

- [`support_relocation`](support_relocation.md)
