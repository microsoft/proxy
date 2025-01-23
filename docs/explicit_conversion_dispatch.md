# Class `explicit_conversion_dispatch`

```cpp
class explicit_conversion_dispatch;  // since 3.2

using conversion_dispatch = explicit_conversion_dispatch;  // since 3.2
```

Class `explicit_conversion_dispatch` models a [dispatch](ProDispatch.md) type for explicit type conversion expressions. It meets the [*ProAccessible* requirements](ProAccessible.md) of applicable types. `conversion_dispatch` is an alias of `explicit_conversion_dispatch`.

## Member Functions

| Name                                                         | Description                                         |
| ------------------------------------------------------------ | --------------------------------------------------- |
| (constructor) [nothrow]                                      | constructs an `explicit_conversion_dispatch` object |
| [`operator()`](explicit_conversion_dispatch/operator_call.md) | invokes the dispatch                                |

## Member Types

| Name                                                   | Description                       |
| ------------------------------------------------------ | --------------------------------- |
| [`accessor`](explicit_conversion_dispatch/accessor.md) | provides accessibility to `proxy` |

## Example

```cpp
#include <iostream>

#include "proxy.h"

struct IntConvertible : pro::facade_builder
    ::add_convention<pro::conversion_dispatch, int() const>
    ::build {};

int main() {
  pro::proxy<IntConvertible> p = pro::make_proxy<IntConvertible, short>(123);  // p holds a short
  std::cout << static_cast<int>(*p) << "\n";  // Prints: "123"
}
```

## See Also

- [class `implicit_conversion_dispatch`](implicit_conversion_dispatch.md)
- [class template `operator_dispatch`](operator_dispatch.md)
