# Class `substitution_dispatch`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v4`  
> Since: 4.0.1

```cpp
class substitution_dispatch;
```

Class `substitution_dispatch` models a [dispatch](../ProDispatch.md) type for `proxy` substitution. It meets the [*ProAccessible* requirements](../ProAccessible.md) of applicable types.

## Member Functions

| Name                             | Description                                  |
| -------------------------------- | -------------------------------------------- |
| (constructor) [nothrow]          | constructs an `substitution_dispatch` object |
| [`operator()`](operator_call.md) | invokes the dispatch                         |

## Member Types

| Name                      | Description                       |
| ------------------------- | --------------------------------- |
| [`accessor`](accessor.md) | provides accessibility to `proxy` |

## Example

```cpp
#include <iostream>

#include <proxy/proxy.h>

struct Runnable : pro::facade_builder                                    //
                  ::add_convention<pro::operator_dispatch<"()">, void()> //
                  ::build {};

struct CopyableRunnable : pro::facade_builder                               //
                          ::support_copy<pro::constraint_level::nontrivial> //
                          ::add_facade<Runnable>                            //
                          ::add_direct_convention<pro::substitution_dispatch,
                                                  pro::proxy<Runnable>() const&,
                                                  pro::proxy<Runnable>() &&> //
                          ::build {};

int main() {
  pro::proxy<CopyableRunnable> p1 = pro::make_proxy<CopyableRunnable>(
      [] { std::cout << "Lambda expression invoked\n"; });

  // Implicit conversion via const reference of pro::proxy<CopyableRunnable>
  pro::proxy<Runnable> p2 = p1;
  std::cout << std::boolalpha << p2.has_value() << "\n"; // Prints "true"

  // Implicit conversion via rvalue reference of pro::proxy<CopyableRunnable>
  pro::proxy<Runnable> p3 = std::move(p1);
  std::cout << p1.has_value() << "\n"; // Prints "false"
  (*p3)();                             // Prints "Lambda expression invoked"

  // Different from implicit_conversion_dispatch, substitution from a null proxy
  // is well-formed
  pro::proxy<Runnable> p4 = p1;
  std::cout << p4.has_value() << "\n"; // Prints "false"
}
```

## See Also

- [`basic_facade_builder::add_facade`](../basic_facade_builder/add_facade.md)
- [alias template `skills::as_view`](../skills_as_view.md)
- [alias template `skills::as_weak`](../skills_as_weak.md)
- [class `implicit_conversion_dispatch`](../implicit_conversion_dispatch/README.md)
