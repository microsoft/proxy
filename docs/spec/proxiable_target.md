# Concept `proxiable_target`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v4`  
> Since: 3.3.0

```cpp
template <class T, class F>
concept proxiable_target = proxiable<observer-ptr<T>, F>;
```

See [`make_proxy_view`](make_proxy_view.md) for the definition of the exposition-only class template *observer-ptr*.

## Example

```cpp
#include <proxy/proxy.h>

struct Runnable : pro::facade_builder                                    //
                  ::add_convention<pro::operator_dispatch<"()">, void()> //
                  ::build {};

int main() {
  auto fun = [] {};
  static_assert(pro::proxiable_target<decltype(fun), Runnable>);
  static_assert(!pro::proxiable_target<int, Runnable>);
}
```

## See Also

- [concept `proxiable`](proxiable.md)
- [concept `inplace_proxiable_target`](inplace_proxiable_target.md)
