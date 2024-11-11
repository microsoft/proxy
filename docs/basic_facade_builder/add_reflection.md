# `basic_facade_builder::add_reflection`

```cpp
template <class R>
using add_reflection = add_indirect_reflection<R>;

template <class R>
using add_indirect_reflection = basic_facade_builder</* see below */>;

template <class R>
using add_direct_reflection = basic_facade_builder</* see below */>;
```

The alias templates `add_reflection`, `add_indirect_reflection` and `add_direct_reflection` of `basic_facade_builder<Cs, Rs, C>` add reflection types to the template parameters. Specifically,

- `add_reflection` is equivalent to `add_indirect_reflection`.
- `add_indirect_reflection` merges an implementation-defined reflection type `R2` into `Rs`, where:
  - `R2::is_direct` is `false`.
  - `typename R2::reflector_type` is `R`.
  - `typename R2::template accessor<F>` is `typename R2::template accessor<F, R2>` if applicable.
- `add_direct_reflection` merges an implementation-defined reflection type `R2` into `Rs`, where:
  - `R2::is_direct` is `true`.
  - `typename R2::reflector_type` is `R`.
  - `typename R2::template accessor<F>` is `typename R2::template accessor<F, R2>` if applicable.

When `Rs` already contains `R2`, the template parameters shall not change.

## Notes

Adding duplicate reflection types is well-defined, whether done directly via `add_reflection`, `add_indirect_reflection`, `add_direct_reflection`, or indirectly via [`add_facade`](add_facade.md). This process does not affect the behavior of [`build`](build.md) at either compile-time or runtime.

## Example

```cpp
#include <iostream>
#include <typeinfo>

#include "proxy.h"

class RttiReflector {
 public:
  template <class T>
  constexpr explicit RttiReflector(std::in_place_type_t<T>) : type_(typeid(T)) {}

  template <class F, class R>
  struct accessor {
    const char* GetTypeName() const noexcept {
      const RttiReflector& self = pro::proxy_reflect<R>(pro::access_proxy<F>(*this));
      return self.type_.name();
    }
  };

 private:
  const std::type_info& type_;
};

struct RttiAware : pro::facade_builder
    ::add_direct_reflection<RttiReflector>
    ::add_indirect_reflection<RttiReflector>
    ::build {};

int main() {
  int a = 123;
  pro::proxy<RttiAware> p = &a;
  std::cout << p.GetTypeName() << "\n";  // Prints: "Pi" (assuming GCC)
  std::cout << p->GetTypeName() << "\n";  // Prints: "i" (assuming GCC)
}
```

## See Also

- [named requirements: *ProReflection*](../ProReflection.md)
