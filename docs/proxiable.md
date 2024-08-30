# Concept `proxiable`

```cpp
template <class P, class F>
concept proxiable = /* see-below */;
```

The concept `proxiable<P, F>` specifies that [`proxy<F>`](proxy.md) can potentially contain a value of type `P`. For a type `P`, if `P` is an incomplete type, the behavior of evaluating `proxiable<P, F>` is undefined. `proxiable<P, F>` is `true` when `F` meets the [*ProFacade* requirements](ProFacade.md) of `P`; otherwise, it is `false`.

## Example

```cpp
#include <string>
#include <vector>

#include "proxy.h"

PRO_DEF_FREE_DISPATCH(FreeToString, std::to_string, ToString);

struct Stringable : pro::facade_builder
    ::add_convention<FreeToString, std::string()>
    ::build {};

int main() {
  static_assert(pro::proxiable<int*, Stringable>);
  static_assert(pro::proxiable<std::shared_ptr<double>, Stringable>);
  static_assert(!pro::proxiable<std::vector<int>*, Stringable>);
}
```

## See Also

- [class template `proxy`](proxy.md)
- [function template `make_proxy`](make_proxy.md)
- [concept `inplace_proxiable_target`](inplace_proxiable_target.md)
