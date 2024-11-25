# Macro `PRO_DEF_WEAK_DISPATCH`

```cpp
#define PRO_DEF_WEAK_DISPATCH // see below
```

Macro `PRO_DEF_WEAK_DISPATCH` defines a "weak dispatch" type with a default implementation. It supports the following syntax:

```cpp
PRO_DEF_WEAK_DISPATCH(dispatch_name, existing_dispatch, default_func_name);
```

Defines a class named `dispatch_name` that inherits `existing_dispatch` and provides additional overloads of `operator()` calling `default_func_name`. Effectively equivalent to:

```cpp
struct dispatch_name : existing_dispatch {
  using existing_dispatch::operator();
  template <class... Args>
  decltype(auto) operator()(std::nullptr_t, Args&&... args)
      noexcept(noexcept(default_func_name(std::forward<Args>(args)...)))
      requires(requires { default_func_name(std::forward<Args>(args)...); }) {
    return default_func_name(std::forward<Args>(args)...);
  }
}
```

## Notes

A "weak dispatch" can extend an existing dispatch with a default implementation that does not depend on the contained value of a `proxy` object. This is useful when instantiating a `proxy<F>` with a value that does not support some conventions defined by `F`. Similar to [`PRO_DEF_FREE_DISPATCH`](PRO_DEF_FREE_DISPATCH.md), `default_func_name` can be the name of an arbitrary function or anything that supports `()` syntax, including a constructor. Compared to wrapping the default implementation with [`PRO_DEF_FREE_DISPATCH`](PRO_DEF_FREE_DISPATCH.md), using "weak dispatch" when applicable can effectively improve compilation speed and binary size, in case some contained value of a `proxy` object does not participate code generation.

In [Java](https://docs.oracle.com/javase/specs/jls/se23/html/jls-9.html#jls-9.4-200) or [C#](https://learn.microsoft.com/dotnet/csharp/language-reference/proposals/csharp-8.0/default-interface-methods), a "default method" can invoke other abstract methods defined in a same `interface`. This pattern is discouraged when using the "Proxy" library because the invocations are not necessarily indirect. If a "default implementation" otherwise needs to observe the contained value of a `proxy` object, it is encouraged to define a separate free function, and subsequently define a dispatch type of it by using [`PRO_DEF_FREE_DISPATCH`](PRO_DEF_FREE_DISPATCH.md) or [`PRO_DEF_FREE_AS_MEM_DISPATCH`](PRO_DEF_FREE_AS_MEM_DISPATCH.md).

## Example

```cpp
#include <iostream>
#include <string>
#include <vector>

#include "proxy.h"

struct NotImplemented {
  explicit NotImplemented(auto&&...) { throw std::runtime_error{ "Not implemented!" }; }

  template <class T>
  operator T() const noexcept { std::terminate(); }  // Or std::unreachable() in C++23
};

PRO_DEF_MEM_DISPATCH(MemAt, at);
PRO_DEF_WEAK_DISPATCH(WeakMemAt, MemAt, NotImplemented);

struct WeakDictionary : pro::facade_builder
    ::add_convention<WeakMemAt, std::string(int index) const>
    ::build {};

int main() {
  std::vector<const char*> v{"hello", "world"};
  pro::proxy<WeakDictionary> p1 = &v;
  std::cout << p1->at(1) << "\n";  // Prints: "world"
  pro::proxy<WeakDictionary> p2 = pro::make_proxy<WeakDictionary>(123);
  try {
    p2->at(1);
  } catch (const std::runtime_error& e) {
    std::cout << e.what() << "\n";  // Prints: "Not implemented!"
  }
}
```

## See Also

- [named requirements *ProDispatch*](ProDispatch.md)
