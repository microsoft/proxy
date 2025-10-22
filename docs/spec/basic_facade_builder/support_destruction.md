# `basic_facade_builder::support_destruction`

```cpp
template <constraint_level CL>
    requires(/* see below */)
using support_destruction = basic_facade_builder</* see below */>;
```

The alias template `support_destruction` of `basic_facade_builder<Cs, Rs, MaxSize, MaxAlign, Copyability, Relocatability, Destructibility>` adds destruction support to the template parameters. After the operation, `Destructibility` becomes `std::max(Destructibility, CL)`. The expression inside `requires` is equivalent to `CL` is a defined enumerator of `constraint_level`.

## Notes

If no destructibility support is applied before specifying [`build`](build.md), the default value of `build::destructibility` is `pro::constraint_level::nothrow`.

## Example

```cpp
#include <type_traits>

#include <proxy/proxy.h>

struct Movable : pro::facade_builder::build {};

struct NonriviallyDestructible
    : pro::facade_builder                                      //
      ::support_relocation<pro::constraint_level::nontrivial>  //
      ::support_destruction<pro::constraint_level::nontrivial> //
      ::build {};

int main() {
  static_assert(std::is_nothrow_destructible_v<pro::proxy<Movable>>);
  static_assert(
      !std::is_nothrow_destructible_v<pro::proxy<NonriviallyDestructible>>);
}
```

## See Also

- [`support_relocation`](support_relocation.md)
