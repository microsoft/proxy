# Function `swap` (`proxy<F>`)

```cpp
friend void swap(proxy& lhs, proxy& rhs) noexcept(noexcept(lhs.swap(rhs)));
```

Overloads the [std::swap](https://en.cppreference.com/w/cpp/algorithm/swap) algorithm for `proxy`. Exchanges the state of `lhs` with that of `rhs`. Effectively calls `lhs.swap(rhs)`.

This function is not visible to ordinary [unqualified](https://en.cppreference.com/w/cpp/language/unqualified_lookup) or [qualified lookup](https://en.cppreference.com/w/cpp/language/qualified_lookup). It can only be found by [argument-dependent lookup](https://en.cppreference.com/w/cpp/language/adl) when `proxy<F>` is an associated class of the arguments.

## Example

```cpp
#include <iostream>
#include <numbers>
#include <string>

#include "proxy.h"

PRO_DEF_FREE_DISPATCH(FreeToString, std::to_string, ToString);

struct Stringable : pro::facade_builder
    ::add_convention<FreeToString, std::string()>
    ::build {};

int main() {
  pro::proxy<Stringable> p0 = pro::make_proxy<Stringable>(123);
  pro::proxy<Stringable> p1 = pro::make_proxy<Stringable>(std::numbers::pi);
  std::cout << ToString(*p0) << "\n";  // Prints "10"
  std::cout << ToString(*p1) << "\n";  // Prints "3.14..."
  std::ranges::swap(p0, p1);  // finds the hidden friend
  std::cout << ToString(*p0) << "\n";  // Prints "3.14..."
  std::cout << ToString(*p1) << "\n";  // Prints "10"
}
```
