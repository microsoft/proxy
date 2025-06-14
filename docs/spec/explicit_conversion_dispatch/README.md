# Class `explicit_conversion_dispatch`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v4`  
> Since: 3.2.0

```cpp
class explicit_conversion_dispatch;

using conversion_dispatch = explicit_conversion_dispatch;
```

Class `explicit_conversion_dispatch` models a [dispatch](../ProDispatch.md) type for explicit type conversion expressions. It meets the [*ProAccessible* requirements](../ProAccessible.md) of applicable types. `conversion_dispatch` is an alias of `explicit_conversion_dispatch`.

## Member Functions

| Name                             | Description                                         |
| -------------------------------- | --------------------------------------------------- |
| (constructor) [nothrow]          | constructs an `explicit_conversion_dispatch` object |
| [`operator()`](operator_call.md) | invokes the dispatch                                |

## Member Types

| Name                                                   | Description                       |
| ------------------------------------------------------ | --------------------------------- |
| [`accessor`](accessor.md) | provides accessibility to `proxy` |

## Example

```cpp
#include <iostream>

#include <proxy/proxy.h>

struct IntConvertible
    : pro::facade_builder                                     //
      ::add_convention<pro::conversion_dispatch, int() const> //
      ::build {};

int main() {
  // p holds a short
  pro::proxy<IntConvertible> p = pro::make_proxy<IntConvertible, short>(123);
  std::cout << static_cast<int>(*p) << "\n"; // Prints "123"
}
```

## See Also

- [class `implicit_conversion_dispatch`](../implicit_conversion_dispatch/README.md)
- [class template `operator_dispatch`](../operator_dispatch/README.md)
