# Macro `PRO_DEF_REFL_ACCESSOR`

```cpp
#define PRO_DEF_REFL_ACCESSOR // see below
```

Macro `PRO_DEF_REFL_ACCESSOR` defines an accessor template within a reflector type providing [accessibility](ProAccessible.md) via a member function. It supports the following syntax:

```cpp
PRO_DEF_REFL_ACCESSOR(accessibility_func_name);
```

Defines a class template named `accessor` with a member function that invokes [`proxy_reflect`](proxy_reflect.md). Effectively equivalent to:

```cpp
template <class F, class R>
struct accessor {
  const auto& accessibility_func_name() const noexcept {
    return pro::proxy_reflect<R>(pro::access_proxy<F>(*this));
  }
}
```

## Notes

When authoring a reflector type, it is recommended to define an accessor template with `PRO_DEF_REFL_ACCESSOR` in its body. However, this is not required, and users may implement their own accessors (e.g. providing accessibility via an operator) on demand.

## Example

```cpp
#include <iostream>
#include <typeinfo>

#include "proxy.h"

class RttiReflector {
 public:
  template <class T>
  constexpr explicit RttiReflector(std::in_place_type_t<T>) : type_(typeid(T)) {}

  PRO_DEF_REFL_ACCESSOR(ReflectRtti);
  const char* GetName() const noexcept { return type_.name(); }

 private:
  const std::type_info& type_;
};

struct RttiAware : pro::facade_builder
    ::add_reflection<RttiReflector>
    ::build {};

int main() {
  pro::proxy<RttiAware> p = pro::make_proxy<RttiAware>(123);
  std::cout << p->ReflectRtti().GetName() << "\n";  // Prints: "i" (assuming GCC)
}
```

## See Also

- [named requirements *ProReflection*](ProReflection.md)
