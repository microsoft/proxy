# `basic_facade_builder::support_copy`

```cpp
template <constraint_level CL>
    requires(/* see below */)
using support_copy = basic_facade_builder</* see below */>;
```

The alias template `support_copy` of `basic_facade_builder<Cs, Rs, MaxSize, MaxAlign, Copyability, Relocatability, Destructibility>` adds copyability support to the template parameters. After the operation, `Copyability` becomes `std::max(Copyability, CL)`. The expression inside `requires` is equivalent to `CL` is a defined enumerator of `constraint_level`.

## Notes

If no copyability support is applied before specifying [`build`](build.md), the default value of `build::copyability` is `pro::constraint_level::none`.

## Example

```cpp
#include <memory>

#include <proxy/proxy.h>

struct Movable : pro::facade_builder::build {};

struct Copyable : pro::facade_builder                               //
                  ::support_copy<pro::constraint_level::nontrivial> //
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
