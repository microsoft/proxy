# `basic_facade_builder::add_reflection`

```cpp
template <class R>
using add_reflection = basic_facade_builder</* see below */>;
```

The alias template `add_reflection` of `basic_facade_builder<Cs, Rs, C>` incorporates reflection types (see [named requirements: *ProReflection*](../ProReflection.md)) into the template parameters. It merges `R` into `Rs` if `Rs` does not already contain `R`.

## Notes

Adding duplicate reflection types is well-defined, whether done directly via `add_reflection` or indirectly via [`add_facade`](add_facade.md). This process does not affect the behavior of [`build`](build.md) at either compile-time or runtime.

## Example

```cpp
#include <iostream>
#include <typeinfo>

#include "proxy.h"

class DebugReflection {
 public:
  template <class P>
  constexpr explicit DebugReflection(std::in_place_type_t<P>)
      : pointer_type_(typeid(P)),
        element_type_(typeid(typename std::pointer_traits<P>::element_type)) {}

  void PrintDebugInfo() const {
    std::cout << "Pointer type: " << pointer_type_.name() << "\n";
    std::cout << "Element type: " << element_type_.name() << "\n";
  }

 private:
  const std::type_info& pointer_type_;
  const std::type_info& element_type_;
};

struct TestFacade : pro::facade_builder
    ::add_reflection<DebugReflection>
    ::build {};

int main() {
  pro::proxy<TestFacade> p1 = std::make_shared<int>(123);
  pro::proxy_reflect<DebugReflection>(p1).PrintDebugInfo();  // Prints: "Pointer type: St10shared_ptrIiE"
                                                             //         "Element type: i" (assuming GCC)

  double v = 3.14;
  pro::proxy<TestFacade> p2 = &v;
  pro::proxy_reflect<DebugReflection>(p2).PrintDebugInfo();  // Prints: "Pointer type: Pd"
                                                             //         "Element type: d" (assuming GCC)
}
```

## See Also

- [named requirements: *ProReflection*](../ProReflection.md)
