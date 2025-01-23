# Class `implicit_conversion_dispatch`

```cpp
class implicit_conversion_dispatch;  // since 3.2
```

Class `implicit_conversion_dispatch` models a [dispatch](ProDispatch.md) type for implicit type conversion expressions. It meets the [*ProAccessible* requirements](ProAccessible.md) of applicable types.

## Member Functions

| Name                                                         | Description                                         |
| ------------------------------------------------------------ | --------------------------------------------------- |
| (constructor) [nothrow]                                      | constructs an `implicit_conversion_dispatch` object |
| [`operator()`](implicit_conversion_dispatch/operator_call.md) | invokes the dispatch                                |

## Member Types

| Name                                                   | Description                       |
| ------------------------------------------------------ | --------------------------------- |
| [`accessor`](implicit_conversion_dispatch/accessor.md) | provides accessibility to `proxy` |

## Example

```cpp
#include <iomanip>
#include <iostream>

#include "proxy.h"

struct Runnable : pro::facade_builder
    ::add_convention<pro::operator_dispatch<"()">, void()>
    ::build {};

struct CopyableRunnable : pro::facade_builder
    ::support_copy<pro::constraint_level::nontrivial>
    ::add_facade<Runnable>
    ::add_direct_convention<pro::implicit_conversion_dispatch,
        pro::proxy<Runnable>() const&, pro::proxy<Runnable>() &&>
    ::build {};

int main() {
  pro::proxy<CopyableRunnable> p1 = pro::make_proxy<CopyableRunnable>(
      [] { std::cout << "Lambda expression invoked\n"; });
  auto p2 = p1;  // Copy construction
  pro::proxy<Runnable> p3 = p2;  // Implicit conversion via const reference of pro::proxy<CopyableRunnable>
  std::cout << std::boolalpha << p2.has_value() << "\n";  // Prints: "true"
  // auto p4 = p3;  // Won't compile because pro::proxy<Runnable> is not copy-constructible
  pro::proxy<Runnable> p5 = std::move(p2);  // Implicit conversion via rvalue reference of pro::proxy<CopyableRunnable>
  std::cout << p2.has_value() << "\n";  // Prints: "false"
  (*p5)();  // Prints: "Lambda expression invoked"
}
```

## See Also

- [class `explicit_conversion_dispatch`](explicit_conversion_dispatch.md)
- [class template `operator_dispatch`](operator_dispatch.md)