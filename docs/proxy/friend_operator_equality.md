# Function `operator==` (`proxy<F>`)

```cpp
friend bool operator==(const proxy& lhs, std::nullptr_t) noexcept;
```

Checks whether `lhs` contains a value by comparing it with `nullptr`. A `proxy` that does not contain a value compares equal to `nullptr`; otherwise, it compares non-equal.

This function is not visible to ordinary [unqualified](https://en.cppreference.com/w/cpp/language/unqualified_lookup) or [qualified lookup](https://en.cppreference.com/w/cpp/language/qualified_lookup). It can only be found by [argument-dependent lookup](https://en.cppreference.com/w/cpp/language/adl) when `proxy<F>` is an associated class of the arguments.

The `!=` operator is [synthesized](https://en.cppreference.com/w/cpp/language/default_comparisons) from `operator==`.

## Return Value

`!lhs.has_value()`.

## Example

```cpp
#include <iostream>

#include "proxy.h"

struct AnyMovable : pro::facade_builder::build {};

int main() {
  pro::proxy<AnyMovable> p;
  std::cout << std::boolalpha << (p == nullptr) << "\n";  // Prints "true"
  std::cout << (p != nullptr) << "\n";  // Prints "false"
  p = std::make_unique<int>(123);
  std::cout << (p == nullptr) << "\n";  // Prints "false"
  std::cout << (p != nullptr) << "\n";  // Prints "true"
}
```

## See Also

- [`has_value`](operator_bool.md)
