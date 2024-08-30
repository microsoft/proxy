# `proxy::operator bool`, `proxy::has_value`

```cpp
explicit operator bool() const noexcept;
bool has_value() const noexcept;
```

Checks whether `*this` contains a value.

## Return Value

`true` if `*this` contains a value, or `false` otherwise.

## Example

```cpp
#include <iostream>

#include "proxy.h"

struct AnyMovable : pro::facade_builder::build {};

int main() {
  pro::proxy<AnyMovable> p;
  std::cout << std::boolalpha << p.has_value() << "\n";  // Prints "false"
  p = pro::make_proxy<AnyMovable>(123);
  std::cout << p.has_value() << "\n";  // Prints "true"
  p = nullptr;
  std::cout << static_cast<bool>(p) << "\n";  // Prints "false"
}
```

## See Also

- [`operator==`](friend_operator_equality.md)
