# `basic_facade_builder::build`

```cpp
using build = /* see below */;
```

Specifies a [facade](facade.md) type deduced from the template parameters of `basic_facade_builder<Cs, Rs, C>`. Specifically,

- `typename build::convention_types` is defined as `Cs`, and
- `typename build::reflection_types` is defined as `Rs`, and
- `build::constraints` is a [core constant expression](https://en.cppreference.com/w/cpp/language/constant_expression) of type [`proxiable_ptr_constraints`](../proxiable_ptr_constraints.md) that defines constraints to the pointer types, and
- `build::constraints.max_size` is `C::max_size` if defined by [`restrict_layout`](restrict_layout.md), otherwise `sizeof(void*) * 2u` when `C::max_size` is *default-size*, and
- `build::constraints.max_align` is `C::max_align` if defined by [`restrict_layout`](restrict_layout.md), otherwise `alignof(void*)` when `C::max_align` is *default-size*, and
- `build::constraints.copyability` is `C::copyability` if defined by [`support_copy`](support_copy.md), otherwise `constraint_level::none` when `C::copyability` is *default-cl*, and
- `build::constraints.relocatability` is `C::relocatability` if defined by [`support_rellocation`](support_relocation.md), otherwise `constraint_level::nothrow` when `C::relocatability` is *default-cl*, and
- `build::constraints.destructibility` is `C::destructibility` if defined by [`support_destruction`](support_destruction.md), otherwise `constraint_level::nothrow` when `C::destructibility` is *default-cl*.

The definition of type `build` makes use of the following exposition-only function:

```cpp
consteval proxiable_ptr_constraints normalize(proxiable_ptr_constraints value) {
  if (value.max_size == default-size)
      { value.max_size = sizeof(void*) * 2u; }
  if (value.max_align == default-size)
      { value.max_align = alignof(void*); }
  if (value.copyability == default-cl)
      { value.copyability = constraint_level::none; }
  if (value.relocatability == default-cl)
      { value.relocatability = constraint_level::nothrow; }
  if (value.destructibility == default-cl)
      { value.destructibility = constraint_level::nothrow; }
  return value;
}
```

## Member Types

| Name               | Definition |
| ------------------ | ---------- |
| `convention_types` | `Cs`       |
| `reflection_types` | `Rs`       |

## Member Constants

| Name                               | Definition     |
| ---------------------------------- | -------------- |
| `constraints` [static] [constexpr] | `normalize(C)` |

## Notes

It is encouraged to inherit `build` with an empty `struct` before specifying a [`proxy`](../proxy.md), rather than `using` or `typedef` the `build` type into an alias, to improve compilation performance.

The default values of the fields of [`proxiable_ptr_constraints`](../proxiable_ptr_constraints.md) are based on our engineering practices. The default values of `max_size` and `max_alignment` are usually sufficient for many implementations of [fancy pointers](https://en.cppreference.com/w/cpp/named_req/Allocator#Fancy_pointers), such as [`std::unique_ptr`](https://en.cppreference.com/w/cpp/memory/unique_ptr), [`std::shared_ptr`](https://en.cppreference.com/w/cpp/memory/shared_ptr), and [boost::interprocess::offset_ptr](https://www.boost.org/doc/libs/1_85_0/doc/html/interprocess/offset_ptr.html). A larger combination of size and alignment ensures better compatibility with the implementation of the underlying pointers and reduces heap allocation when the element type fits in the buffer (see [function template `make_proxy`](../make_proxy.md)), at the cost of making the corresponding [`proxy`](../proxy.md) objects larger.

## Example

```cpp
#include <type_traits>

#include "proxy.h"

struct DefaultBase : pro::facade_builder
    ::build {};

struct CopyableBase : pro::facade_builder
    ::support_copy<pro::constraint_level::nontrivial>
    ::build {};

struct TrivialBase : pro::facade_builder
    ::support_copy<pro::constraint_level::trivial>
    ::support_relocation<pro::constraint_level::trivial>
    ::support_destruction<pro::constraint_level::trivial>
    ::restrict_layout<sizeof(void*)>
    ::build {};

int main() {
  static_assert(!std::is_copy_constructible_v<pro::proxy<DefaultBase>>);
  static_assert(std::is_nothrow_move_constructible_v<pro::proxy<DefaultBase>>);
  static_assert(std::is_nothrow_destructible_v<pro::proxy<DefaultBase>>);

  static_assert(std::is_copy_constructible_v<pro::proxy<CopyableBase>>);
  static_assert(std::is_nothrow_move_constructible_v<pro::proxy<CopyableBase>>);
  static_assert(std::is_nothrow_destructible_v<pro::proxy<CopyableBase>>);

  static_assert(std::is_trivially_copy_constructible_v<pro::proxy<TrivialBase>>);
  static_assert(std::is_trivially_move_constructible_v<pro::proxy<TrivialBase>>);
  static_assert(std::is_trivially_destructible_v<pro::proxy<TrivialBase>>);
}
```

## See Also

- [class template `proxy`](../proxy.md)
