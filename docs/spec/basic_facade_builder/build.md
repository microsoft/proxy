# `basic_facade_builder::build`

```cpp
using build = /* see below */;
```

Specifies a [facade](../facade.md) type deduced from the template parameters of `basic_facade_builder<Cs, Rs, C>`.

## Member Types

| Name               | Definition |
| ------------------ | ---------- |
| `convention_types` | `Cs`       |
| `reflection_types` | `Rs`       |

## Member Constants

| Name                                   | Definition                                                   |
| -------------------------------------- | ------------------------------------------------------------ |
| `max_size` [static] [constexpr]        | `MaxSize == default-size ? sizeof(void*) * 2u : MaxSize`     |
| `max_align` [static] [constexpr]       | `MaxAlign == default-size ? alignof(void*) : MaxAlign`       |
| `copyability` [static] [constexpr]     | `Copyability == default-cl ? constraint_level::none : Copyability` |
| `relocatability` [static] [constexpr]  | `Relocatability == default-cl ? constraint_level::trivial : Relocatability` |
| `destructibility` [static] [constexpr] | `Destructibility == default-cl ? constraint_level::nothrow : Destructibility` |

## Notes

It is encouraged to inherit `build` with an empty `struct` before specifying a [`proxy`](../proxy/README.md), rather than `using` or `typedef` the `build` type into an alias, to improve compilation performance.

The default values of the member constants are based on our engineering practices. The default values of `max_size` and `max_alignment` are usually sufficient for many implementations of [fancy pointers](https://en.cppreference.com/w/cpp/named_req/Allocator#Fancy_pointers), such as [`std::unique_ptr`](https://en.cppreference.com/w/cpp/memory/unique_ptr), [`std::shared_ptr`](https://en.cppreference.com/w/cpp/memory/shared_ptr), and [`boost::interprocess::offset_ptr`](https://www.boost.org/doc/libs/1_85_0/doc/html/interprocess/offset_ptr.html). A larger combination of size and alignment ensures better compatibility with the implementation of the underlying pointers and reduces heap allocation when the element type fits in the buffer (see [function template `make_proxy`](../make_proxy.md)), at the cost of making the corresponding [`proxy`](../proxy/README.md) objects larger.

## Example

```cpp
#include <type_traits>

#include <proxy/proxy.h>

struct DefaultBase : pro::facade_builder //
                     ::build {};

struct CopyableBase : pro::facade_builder                               //
                      ::support_copy<pro::constraint_level::nontrivial> //
                      ::build {};

struct TrivialBase : pro::facade_builder                                   //
                     ::support_copy<pro::constraint_level::trivial>        //
                     ::support_relocation<pro::constraint_level::trivial>  //
                     ::support_destruction<pro::constraint_level::trivial> //
                     ::restrict_layout<sizeof(void*)>                      //
                     ::build {};

int main() {
  static_assert(!std::is_copy_constructible_v<pro::proxy<DefaultBase>>);
  static_assert(std::is_nothrow_move_constructible_v<pro::proxy<DefaultBase>>);
  static_assert(std::is_nothrow_destructible_v<pro::proxy<DefaultBase>>);

  static_assert(std::is_copy_constructible_v<pro::proxy<CopyableBase>>);
  static_assert(std::is_nothrow_move_constructible_v<pro::proxy<CopyableBase>>);
  static_assert(std::is_nothrow_destructible_v<pro::proxy<CopyableBase>>);

  static_assert(
      std::is_trivially_copy_constructible_v<pro::proxy<TrivialBase>>);
  static_assert(
      std::is_trivially_move_constructible_v<pro::proxy<TrivialBase>>);
  static_assert(std::is_trivially_destructible_v<pro::proxy<TrivialBase>>);
}
```

## See Also

- [class template `proxy`](../proxy/README.md)
