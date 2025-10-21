# Class template `basic_facade_builder`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v4`

The definitions of `basic_facade_builder` and `facade_builder` make use of the following exposition-only constants:

```cpp
constexpr std::size_t default-size = std::numeric_limits<std::size_t>::max(); // exposition only
constexpr constraint_level default-cl = static_cast<constraint_level>(
    std::numeric_limits<std::underlying_type_t<constraint_level>>::min()); // exposition only
```

Given a [facade](../facade.md) type `F`, any meaningful value of `F::max_size` and `F::max_align` is less than *default-size*; any meaningful value of `F::copyability`, `F::relocatability`, and `F::destructibility` is greater than *default-cl*.

```cpp
template <class Cs, class Rs, std::size_t MaxSize, std::size_t MaxAlign,
          constraint_level Copyability, constraint_level Relocatability,
          constraint_level Destructibility>
class basic_facade_builder;

using facade_builder =
    basic_facade_builder<std::tuple<>, std::tuple<>, default-size, default-size,
                         default-cl, default-cl, default-cl>;
```

`basic_facade_builder` provides a member type `build` that compiles the template parameters into a [`facade`](../facade.md) type. The template parameters can be modified via various member alias templates that specify `basic_facade_builder` with the modified template parameters.

## Member Types

| Name                | Description                                                  |
| ------------------- | ------------------------------------------------------------ |
| [`build`](build.md) | Specifies a [`facade`](../facade.md) type deduced from the template parameters of the `basic_facade_builder` |

## Member Alias Templates

| Name                                                         | Description                                                  |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| [`add_convention`<br />`add_indirect_convention`<br />`add_direct_convention`](add_convention.md) | Adds a convention to the template parameters                 |
| [`add_facade`](add_facade.md)                                | Adds a facade to the template parameters                     |
| [`add_reflection`<br />`add_indirect_reflection`<br />`add_direct_reflection`](add_reflection.md) | Adds a reflection to the template parameters                 |
| [`add_skill`](add_skill.md)                                  | Adds a custom skill                                          |
| [`restrict_layout`](restrict_layout.md)                      | Specifies maximum `MaxSize` and `MaxAlign` in the template parameters |
| [`support_copy`](support_copy.md)                            | Specifies minimum `Copyability` in the template parameters   |
| [`support_destruction`](support_destruction.md)              | Specifies minimum `Destructibility` in the template parameters |
| [`support_relocation`](support_relocation.md)                | Specifies minimum `Relocatability` in the template parameters |

## Member Functions

| Name                    | Description                               |
| ----------------------- | ----------------------------------------- |
| (constructor) [deleted] | Has neither default nor copy constructors |

## Notes

The design of `basic_facade_builder` utilizes template metaprogramming techniques. We recommend the following guidelines when using `basic_facade_builder` to define a facade type.

- **Define a type for each facade.**

For example, when defining a `Formattable` facade, the following two definitions are both syntactically correct:

```cpp
// (1) Recommended
struct Formattable : pro::facade_builder
    ::add_skill<pro::skills::format>
    ::build {};

// (2) Discouraged
using Formattable = pro::facade_builder
    ::add_skill<pro::skills::format>
    ::build;
```

Definition `(2)` is a type alias, its "real" type may have a long name, and the type evaluation may be executed for multiple times even when compiling a single source file. Although the two type definitions are equivalent at runtime, definitions like `(2)` may significantly reduce compilation performance. Therefore, it is recommended always to define a facade as a type with inheritance, similar to definition `(1)`.

- **Use the `template` keyword on demand when defining a facade template.**

Consider the following facade template definitions:

```cpp
template <class... Os>
struct MovableCallable : pro::facade_builder
    ::add_convention<pro::operator_dispatch<"()">, Os...>
    ::build {};

template <class... Os>
struct CopyableCallable : pro::facade_builder
    ::add_facade<MovableCallable<Os...>>
    ::support_copy<pro::constraint_level::nontrivial>
    ::build {};
```

Although GCC can usually compile the code above, it does not adhere to the C++ standard syntax, and as a result, it won't compile with Clang or MSVC ([live demo](https://godbolt.org/z/38ce4jb8a)). This is because type `add_facade<MovableCallable<Os...>>` depends on the template parameters, and an explicit `template` is required when specifying its member alias template `support_copy`. To fix the code, we could either add the keyword `template` before `support_copy`, or simply swap `add_facade` and `support_copy`. For instance:

```cpp
template <class... Os>
struct CopyableCallable : pro::facade_builder
    ::add_facade<MovableCallable<Os...>>
    ::template support_copy<pro::constraint_level::nontrivial>
    ::build {};

// Or

template <class... Os>
struct CopyableCallable : pro::facade_builder
    ::support_copy<pro::constraint_level::nontrivial>
    ::add_facade<MovableCallable<Os...>>
    ::build {};
```

## Example

```cpp
#include <iostream>

#include <proxy/proxy.h>

template <class... Overloads>
struct MovableCallable
    : pro::facade_builder                                          //
      ::add_convention<pro::operator_dispatch<"()">, Overloads...> //
      ::build {};

template <class... Overloads>
struct CopyableCallable : pro::facade_builder                               //
                          ::support_copy<pro::constraint_level::nontrivial> //
                          ::add_facade<MovableCallable<Overloads...>>       //
                          ::build {};

// MyFunction has similar functionality as std::function but supports multiple
// overloads MyMoveOnlyFunction has similar functionality as
// std::move_only_function but supports multiple overloads
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
  (*p0)(123); // Prints "f() called. Args: 123:i," (assuming GCC)
  MyMoveOnlyFunction<void(), void(int), void(double)> p1{&f};
  (*p1)();    // Prints "f() called. Args:"
  (*p1)(456); // Prints "f() called. Args: 456:i,"
  (*p1)(1.2); // Prints "f() called. Args: 1.2:d,"
}
```

## See Also

- [concept `facade`](../facade.md)
- [macro `PRO_DEF_MEM_DISPATCH`](../PRO_DEF_MEM_DISPATCH.md)
