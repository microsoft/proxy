# `basic_facade_builder::support_rtti`<br />`basic_facade_builder::support_indirect_rtti`<br />`basic_facade_builder::support_direct_rtti`

```cpp
using support_rtti = support_indirect_rtti;

using support_indirect_rtti = basic_facade_builder</* see below */>;

using support_direct_rtti = basic_facade_builder</* see below */>;
```

The member types `support_rtti`, `support_indirect_rtti` and `support_direct_rtti` add necessary convention and reflection types to the template parameters, enabling [RTTI](https://en.wikipedia.org/wiki/Run-time_type_information) support for [`proxy<F>`](../proxy.md), where `F` is a [facade](../facade.md) type built from `basic_facade_builder`. For an RTTI-enabled facade `F`, non-member functions `proxy_typeid` (similar to [`std::any::type`](https://en.cppreference.com/w/cpp/utility/any/type)) and `proxy_cast` (similar to [`std::any_cast`](https://en.cppreference.com/w/cpp/utility/any/any_cast)) are available for [`proxy_indirect_accessor<F>`](../proxy_indirect_accessor.md) (if `support_rtti` or `support_indirect_rtti` is specified) or [`proxy<F>`](../proxy.md) (if `support_direct_rtti` is specified).

## Non-Member Functions

| Name                                           | Description                                |
| ---------------------------------------------- | ------------------------------------------ |
| [`proxy_typeid`](support_rtti/proxy_typeid.md) | returns the `typeid` of the contained type |
| [`proxy_cast`](support_rtti/proxy_cast.md)     | type-safe access to the contained object   |

## Example

```cpp
#include <iostream>

#include "proxy.h"

struct RttiAware : pro::facade_builder
    ::support_rtti
    ::support_direct_rtti
    ::build {};

int main() {
  int v = 123;
  pro::proxy<RttiAware> p = &v;
  std::cout << proxy_typeid(p).name() << "\n";  // Prints: "Pi" (assuming GCC)
  std::cout << proxy_cast<int*>(p) << "\n";  // Prints the address of v
  std::cout << proxy_typeid(*p).name() << "\n";  // Prints: "i" (assuming GCC)
  std::cout << proxy_cast<int>(*p) << "\n";  // Prints: "123"
}
```

## See Also

- [`support_view`](support_view.md)
