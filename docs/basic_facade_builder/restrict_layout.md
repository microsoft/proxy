# `basic_facade_builder::restrict_layout`

```cpp
template <std::size_t PtrSize, std::size_t PtrAlign = /* see below */>
    requires(std::has_single_bit(PtrAlign) && PtrSize % PtrAlign == 0u)
using restrict_layout = basic_facade_builder</* see below */>;
```

The alias template `restrict_layout` of `basic_facade_builder<Cs, Rs, C>` adds layout restrictions to the template parameters, specifically `C::max_size` and `C::max_align`. The default value of `PtrAlign` is the maximum possible alignment of an object of size `PtrSize`, not greater than `alignof(std::max_align_t`). After applying the restriction, `C::max_size` becomes `std::min(C::max_size, PtrSize)`, and `C::max_align` becomes `std::min(C::max_align, PtrAlign)`.

## Notes

If no layout restriction is applied before specifying [`build`](build.md), the default value of `build::constraints.max_size` is `sizeof(void*) * 2`, and the default value of `build::constraints.max_align` is `alignof(void*)`.

## Example

```cpp
#include <array>
#include <memory>

#include "proxy.h"

struct DefaultFacade : pro::facade_builder::build {};

struct SmallFacade : pro::facade_builder
    ::restrict_layout<sizeof(void*)>
    ::build {};

int main() {
  static_assert(sizeof(pro::proxy<DefaultFacade>) > sizeof(pro::proxy<SmallFacade>));
  static_assert(pro::proxiable<std::unique_ptr<int>, DefaultFacade>);
  static_assert(pro::proxiable<std::unique_ptr<int>, SmallFacade>);
  static_assert(pro::proxiable<std::shared_ptr<int>, DefaultFacade>);
  static_assert(!pro::proxiable<std::shared_ptr<int>, SmallFacade>);
  static_assert(pro::inplace_proxiable_target<std::array<int*, 2>, DefaultFacade>);
  static_assert(!pro::inplace_proxiable_target<std::array<int*, 2>, SmallFacade>);
  static_assert(!pro::inplace_proxiable_target<std::array<int*, 3>, DefaultFacade>);
  static_assert(!pro::inplace_proxiable_target<std::array<int*, 3>, SmallFacade>);
  pro::proxy<DefaultFacade> p1 = std::make_shared<int>(123);
  // pro::proxy<SmallFacade> p2 = std::make_shared<int>(123);  // Won't compile
}
```

## See Also

- [`build`](build.md)
- [concept `inplace_proxiable_target`](../inplace_proxiable_target.md)
