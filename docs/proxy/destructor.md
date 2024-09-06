# `proxy<F>::~proxy`

```cpp
~proxy() requires(F::constraints.destructibility == constraint_level::trivial)
    = default;
~proxy() noexcept(F::constraints.destructibility == constraint_level::nothrow)
    requires(F::constraints.destructibility == constraint_level::nontrivial ||
        F::constraints.destructibility == constraint_level::nothrow);
```

Destroys the `proxy` object. If the `proxy` contains a value, the contained value is also destroyed. The destructor is trivial when `F::constraints.destructibility` is `constraint_level::trivial`.

## Example

```cpp
#include <cstdio>

#include "proxy.h"

struct AnyMovable : pro::facade_builder::build {};

struct Foo {
  ~Foo() { puts("Destroy Foo"); }
};

int main() {
  pro::proxy<AnyMovable> p = pro::make_proxy<AnyMovable, Foo>();
}  // The destructor of `Foo` is called when `p` is destroyed
```
