# `basic_facade_builder::add_convention`, `basic_facade_builder::add_direct_convention`, `basic_facade_builder::add_indirect_convention`

```cpp
template <class D, class... Os> requires(/* see below */)
using add_convention = add_indirect_convention<D, Os...>;

template <class D, class... Os> requires(/* see below */)
using add_indirect_convention = basic_facade_builder</* see below */>;

template <class D, class... Os> requires(/* see below */)
using add_direct_convention = basic_facade_builder</* see below */>;
```

The alias templates `add_convention`, `add_indirect_convention`, and `add_direct_convention` of `basic_facade_builder<Cs, Rs, C>` add convention types to the template parameters. The expression inside `requires` is equivalent to `sizeof...(Os) > 0u` and each type in `Os` meets the [*ProOverload* requirements](../ProOverload.md). Let `F` be a facade type,

- `add_convention` is equivalent to `add_indirect_convention`.
- `add_indirect_convention` merges an implementation-defined convention type `IC` into `Cs`, where:
  - `IC::is_direct` is `false`.
  - `typename IC::dispatch_type` is `D`.
  - `typename IC::overload_types` is a [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type of distinct types in `Os`.
  - `typename IC::template accessor<F>` is `typename D::template accessor<F, IC, Os...>` if applicable.
- `add_direct_convention` merges an implementation-defined convention type `IC` into `Cs`, where:
  - `IC::is_direct` is `true`.
  - `typename IC::dispatch_type` is `D`.
  - `typename IC::overload_types` is a [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type of distinct types in `Os`.
  - `typename IC::template accessor<F>` is `typename D::template accessor<F, IC, Os...>` if applicable.

When `Cs` already contains a convention type `IC2` where `IC2::is_direct == IC::is_direct && std::is_same_v<typename IC2::dispatch_type, typename IC::dispatch_type>` is `true`, `Os` merges with `typename IC2::overload_types` and removes duplicates, and `std::tuple_size_v<Cs>` shall not change.

## Notes

Adding duplicated combinations of some dispatch type and overload type is well-defined (either directly via `add_convention`, `add_indirect_convention`, `add_direct_convention`, or indirectly via [`add_facade`](add_facade.md)), and does not have side-effects to [`build`](build.md) at either compile-time or runtime.

## Example

```cpp
#include <iostream>
#include <memory>
#include <string>

#include "proxy.h"

PRO_DEF_FREE_DISPATCH(FreeToString, std::to_string, ToString);

struct BasicStringable : pro::facade_builder
    ::add_convention<FreeToString, std::string() const>
    ::build {};

struct Stringable : pro::facade_builder
    ::add_facade<BasicStringable>
    ::support_copy<pro::constraint_level::nontrivial>
    ::add_direct_convention<pro::conversion_dispatch<pro::proxy<BasicStringable>>,
        pro::proxy<BasicStringable>() &&>
    ::build {};

int main() {
  pro::proxy<Stringable> p1 = std::make_shared<int>(123);
  pro::proxy<Stringable> p2 = p1;
  pro::proxy<BasicStringable> p3 = static_cast<pro::proxy<BasicStringable>>(std::move(p2));
  pro::proxy<BasicStringable> p4 = std::move(p3);
  // pro::proxy<BasicStringable> p5 = p4; // Won't compile
  std::cout << ToString(*p4) << "\n";  // Prints: "123"
  std::cout << std::boolalpha << p3.has_value() << "\n";  // Prints: "false"
}
```

## See Also

- [macro `PRO_DEF_MEM_DISPATCH`](../PRO_DEF_MEM_DISPATCH.md)
- [macro `PRO_DEF_FREE_DISPATCH`](../PRO_DEF_FREE_DISPATCH.md)
- [class template `operator_dispatch`](../operator_dispatch.md)
- [class template `conversion_dispatch`](../conversion_dispatch.md)
- [macro `PRO_DEF_WEAK_DISPATCH`](../PRO_DEF_WEAK_DISPATCH.md)
