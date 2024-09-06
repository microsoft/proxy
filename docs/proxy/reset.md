# `proxy::reset`

```cpp
void reset()
    noexcept(F::constraints.destructibility >= constraint_level::nothrow)
    requires(F::constraints.destructibility >= constraint_level::nontrivial);
```

Destroys the contained value if it exists. After the call, `*this` does not contain a value.

## Example

```cpp
#include <iostream>

#include "proxy.h"

struct AnyMovable : pro::facade_builder::build {};

int main() {
  pro::proxy<AnyMovable> p = pro::make_proxy<AnyMovable>(123);
  std::cout << std::boolalpha << p.has_value() << "\n";  // Prints "true"
  p.reset();
  std::cout << p.has_value() << "\n";  // Prints "false"
}
```

## See Also

- [`operator=`](assignment.md)
