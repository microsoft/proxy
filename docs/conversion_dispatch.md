# Class template `conversion_dispatch`

```cpp
template <class T, bool Expl = true>
class conversion_dispatch;
```

Class template `conversion_dispatch` is a [dispatch](ProDispatch.md) type for explicit or implicit type conversion expressions. It meets the [*ProAccessible* requirements](ProAccessible.md) of applicable types. `T` is the target type for conversion, and `Expl` specifies whether the conversion is explicit (defaults to `true`, as recommended in [C++ Core Guidelines C.164: Avoid implicit conversion operators](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c164-avoid-implicit-conversion-operators)).

## Member Functions

| Name                                                 | Description                               |
| ---------------------------------------------------- | ----------------------------------------- |
| (constructor) [trivial]                              | constructs a `conversion_dispatch` object |
| [`operator()`](conversion_dispatch/operator_call.md) | invokes the dispatch                      |

## Member Types

| Name                                          | Description                       |
| --------------------------------------------- | --------------------------------- |
| [`accessor`](conversion_dispatch/accessor.md) | provides accessibility to `proxy` |

## Example

```cpp
#include <iomanip>
#include <iostream>

#include "proxy.h"

struct DoubleConvertible : pro::facade_builder
    ::add_convention<pro::conversion_dispatch<double>, double() const>
    ::build {};

struct Runnable : pro::facade_builder
    ::add_convention<pro::operator_dispatch<"()">, void()>
    ::build {};

struct CopyableRunnable : pro::facade_builder
    ::support_copy<pro::constraint_level::nontrivial>
    ::add_facade<Runnable>
    ::add_direct_convention<pro::conversion_dispatch<pro::proxy<Runnable>, false>,
        pro::proxy<Runnable>() const&, pro::proxy<Runnable>() &&>
    ::build {};

int main() {
  // Explicit conversion
  pro::proxy<DoubleConvertible> p1 = pro::make_proxy<DoubleConvertible>(123);  // p1 holds an integer
  std::cout << std::fixed << std::setprecision(10) << std::boolalpha;
  std::cout << static_cast<double>(*p1) << "\n";  // Prints: "123.0000000000"

  // Implicit conversion
  pro::proxy<CopyableRunnable> p2 = pro::make_proxy<CopyableRunnable>(
      [] { std::cout << "Lambda expression invoked\n"; });
  auto p3 = p2;  // Copy construction
  pro::proxy<Runnable> p4 = p3;  // Implicit conversion via const reference of pro::proxy<CopyableRunnable>
  std::cout << p3.has_value() << "\n";  // Prints: "true"
  // auto p5 = p4;  // Won't compile because pro::proxy<Runnable> is not copy-constructible
  pro::proxy<Runnable> p6 = std::move(p3);  // Implicit conversion via rvalue reference of pro::proxy<CopyableRunnable>
  std::cout << p3.has_value() << "\n";  // Prints: "false"
  (*p6)();  // Prints: "Lambda expression invoked"
}
```

## See Also

- [class template `operator_dispatch`](operator_dispatch.md)
