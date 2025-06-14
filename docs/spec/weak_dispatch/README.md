# Class template `weak_dispatch`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v4`  
> Since: 3.2.0

```cpp
template <class D>
struct weak_dispatch : D;
```

Class template `weak_dispatch<D>` extends an existing [dispatch](../ProDispatch.md) type `D` and offers a default implementation of `operator()` that throws [`not_implemented`](../not_implemented.md) when a convention is not implemented in `D`.

## Member Functions

| Name                                           | Description                         |
| ---------------------------------------------- | ----------------------------------- |
| (constructor)                                  | constructs a `weak_dispatch` object |
| (destructor)                                   | destroys a `weak_dispatch` object   |
| [`operator()`](operator_call.md) | invokes the dispatch                |

## Notes

A "weak dispatch" can extend an existing dispatch with a default implementation that does not depend on the contained value of a `proxy` object. This is useful when instantiating a `proxy<F>` with a value that does not support some conventions defined by `F`. Compared to wrapping the default implementation with [`PRO_DEF_FREE_DISPATCH`](../PRO_DEF_FREE_DISPATCH.md), using "weak dispatch" when applicable can effectively improve compilation speed and binary size, in case some contained value of a `proxy` object does not participate code generation.

In [Java](https://docs.oracle.com/javase/specs/jls/se23/html/jls-9.html#jls-9.4-200) or [C#](https://learn.microsoft.com/dotnet/csharp/language-reference/proposals/csharp-8.0/default-interface-methods), a "default method" can invoke other abstract methods defined in a same `interface`. This pattern is discouraged when using the Proxy library because the invocations are not necessarily indirect. If a "default implementation" otherwise needs to observe the contained value of a `proxy` object, it is encouraged to define a separate free function, and subsequently define a dispatch type of it by using [`PRO_DEF_FREE_DISPATCH`](../PRO_DEF_FREE_DISPATCH.md) or [`PRO_DEF_FREE_AS_MEM_DISPATCH`](../PRO_DEF_FREE_AS_MEM_DISPATCH.md).

## Example

```cpp
#include <iostream>
#include <string>
#include <vector>

#include <proxy/proxy.h>

PRO_DEF_MEM_DISPATCH(MemAt, at);

struct WeakDictionary : pro::facade_builder //
                        ::add_convention<pro::weak_dispatch<MemAt>,
                                         std::string(int index) const> //
                        ::build {};

int main() {
  std::vector<const char*> v{"hello", "world"};
  pro::proxy<WeakDictionary> p1 = &v;
  std::cout << p1->at(1) << "\n"; // Prints "world"
  pro::proxy<WeakDictionary> p2 = pro::make_proxy<WeakDictionary>(123);
  try {
    p2->at(1);
  } catch (const pro::not_implemented& e) {
    std::cout << e.what() << "\n"; // Prints an explanatory string
  }
}
```

## See Also

- [named requirements *ProDispatch*](../ProDispatch.md)
