# Class template `basic_facade_builder`

The definitions of `basic_facade_builder` and `facade_builder` make use of the following exposition-only constants:

```cpp
constexpr std::size_t default-size = std::numeric_limits<std::size_t>::max(); // exposition only
constexpr constraint_level default-cl = static_cast<constraint_level>(
    std::numeric_limits<std::underlying_type_t<constraint_level>>::min()); // exposition only
```

*default-size* and *default-cl* denote that a field in [`proxiable_ptr_constraints`](proxiable_ptr_constraints.md) is not specified in the template parameters of a `basic_facade_builder` specialization. In an instantiation of `proxiable_ptr_constraints`, any meaningful value of `max_size` and `max_align` is less than *default-size*; any meaningful value of `copyability`, `relocatability`, and `destructibility` is greater than *default-cl*.

```cpp
template <class Cs, class Rs, proxiable_ptr_constraints C>
class basic_facade_builder;

using facade_builder = basic_facade_builder<std::tuple<>, std::tuple<>,
    proxiable_ptr_constraints{
        .max_size = default-size,
        .max_align = default-size,
        .copyability = default-cl,
        .relocatability = default-cl,
        .destructibility = default-cl}>;
```

`class Cs`, `class Rs`, and `proxiable_ptr_constraints C` are the template parameters of `basic_facade_builder`. `basic_facade_builder` provides a member type `build` that compiles the template parameters into a [`facade`](facade.md) type. The template parameters can be modified via various member alias templates that specify `basic_facade_builder` with the modified template parameters.

## Member Types

| Name                                     | Description                                                  |
| ---------------------------------------- | ------------------------------------------------------------ |
| [`build`](basic_facade_builder/build.md) | Specifies a [`facade`](facade.md) type deduced from the template parameters of the `basic_facade_builder` |

## Member Alias Templates

| Name                                                         | Description                                                  |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| [`add_convention`<br />`add_indirect_convention`<br />`add_direct_convention`](basic_facade_builder/add_convention.md) | Adds a convention to the template parameters                 |
| [`add_reflection`<br />`add_indirect_reflection`<br />`add_direct_reflection`](basic_facade_builder/add_reflection.md) | Adds a reflection to the template parameters                 |
| [`add_facade`](basic_facade_builder/add_facade.md)           | Adds a facade to the template parameters                     |
| [`restrict_layout`](basic_facade_builder/restrict_layout.md) | Specifies maximum `max_size` and `max_align` of `C` in the template parameters |
| [`support_copy`](basic_facade_builder/support_copy.md)       | Specifies minimum `copyability` of `C` in the template parameters |
| [`support_relocation`](basic_facade_builder/support_relocation.md) | Specifies minimum `relocatability` of `C` in the template parameters |
| [`support_destruction`](basic_facade_builder/support_destruction.md) | Specifies minimum `destructibility` of `C` in the template parameters |

## Member Functions

| Name                    | Description                               |
| ----------------------- | ----------------------------------------- |
| (constructor) [deleted] | Has neither default nor copy constructors |

## Example

```cpp
#include <iostream>

#include "proxy.h"

template <class... Overloads>
struct MovableCallable : pro::facade_builder
    ::add_convention<pro::operator_dispatch<"()">, Overloads...>
    ::build {};

template <class... Overloads>
struct CopyableCallable : pro::facade_builder
    ::support_copy<pro::constraint_level::nontrivial>
    ::add_facade<MovableCallable<Overloads...>>
    ::build {};

// MyFunction has similar functionality as std::function but supports multiple overloads
// MyMoveOnlyFunction has similar functionality as std::move_only_function but supports multiple overloads
template <class... Overloads>
using MyFunction = pro::proxy<CopyableCallable<Overloads...>>;
template <class... Overloads>
using MyMoveOnlyFunction = pro::proxy<MovableCallable<Overloads...>>;

int main() {
  auto f = [](auto&&... v) {
    std::cout << "f() called. Args: ";
    ((std::cout << v << ":" << typeid(decltype(v)).name() << ", "), ...);
    std::cout << "\n";
  };
  MyFunction<void(int)> p0{&f};
  (*p0)(123);  // Prints "f() called. Args: 123:i," (assuming GCC)
  MyMoveOnlyFunction<void(), void(int), void(double)> p1{&f};
  (*p1)();  // Prints "f() called. Args:"
  (*p1)(456);  // Prints "f() called. Args: 456:i,"
  (*p1)(1.2);  // Prints "f() called. Args: 1.2:d,"
}
```

## See Also

- [concept `facade`](facade.md)
- [macro `PRO_DEF_MEM_DISPATCH`](PRO_DEF_MEM_DISPATCH.md)
