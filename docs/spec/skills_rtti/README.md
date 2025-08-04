# Alias template `rtti`<br />Alias template `indirect_rtti`<br />Alias template `direct_rtti`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v4::skills`  
> Since: 4.0.0

```cpp
template <class FB>
using indirect_rtti = /* see below */;

template <class FB>
using direct_rtti = /* see below */;

template <class FB>
using rtti = indirect_rtti<FB>;
```

The alias templates `rtti`, `indirect_rtti`, and `direct_rtti` modify a specialization of [`basic_facade_builder`](../basic_facade_builder/README.md), enabling [RTTI](https://en.wikipedia.org/wiki/Run-time_type_information) support for [`proxy<F>`](../proxy/README.md), where `F` is a built [facade](../facade.md) type. For an RTTI-enabled facade `F`, non-member functions `proxy_typeid` (similar to [`std::any::type`](https://en.cppreference.com/w/cpp/utility/any/type)) and `proxy_cast` (similar to [`std::any_cast`](https://en.cppreference.com/w/cpp/utility/any/any_cast)) are available for [`proxy_indirect_accessor<F>`](../proxy_indirect_accessor.md) (for `rtti` and `indirect_rtti`) or [`proxy<F>`](../proxy/README.md) (for `direct_rtti`).

## Non-Member Functions

| Name                              | Description                                |
| --------------------------------- | ------------------------------------------ |
| [`proxy_typeid`](proxy_typeid.md) | returns the `typeid` of the contained type |
| [`proxy_cast`](proxy_cast.md)     | type-safe access to the contained object   |

## Example

```cpp
#include <iostream>

#include <proxy/proxy.h>

struct RttiAware : pro::facade_builder                   //
                   ::add_skill<pro::skills::rtti>        //
                   ::add_skill<pro::skills::direct_rtti> //
                   ::build {};

int main() {
  int v = 123;
  pro::proxy<RttiAware> p = &v;
  std::cout << proxy_typeid(p).name() << "\n";  // Prints "Pi" (assuming GCC)
  std::cout << proxy_cast<int*>(p) << "\n";     // Prints the address of v
  std::cout << proxy_typeid(*p).name() << "\n"; // Prints "i" (assuming GCC)
  std::cout << proxy_cast<int>(*p) << "\n";     // Prints "123"
}
```

## See Also

- [`basic_facade_builder::add_skill`](../basic_facade_builder/add_skill.md)
- [`basic_facade_builder::add_reflection`](../basic_facade_builder/add_reflection.md)
