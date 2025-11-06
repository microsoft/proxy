# Class template `is_bitwise_trivially_relocatable`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v4`  
> Since: 4.0.0

```cpp
template <class T>
struct is_bitwise_trivially_relocatable;

template <class T>
constexpr bool is_bitwise_trivially_relocatable_v =
    is_bitwise_trivially_relocatable<T>::value;
```

The class template `is_bitwise_trivially_relocatable<T>` is a type trait whose `value` is `true` when objects of (complete) type `T` can be *bitwise trivially relocated*: a new object of type `T` can be created at an arbitrary suitably aligned storage location by performing a raw byte-wise copy (as if by `std::memcpy`) of `sizeof(T)` bytes from the original object's storage, and the original object can then be considered destroyed (its lifetime ends) without invoking its destructor. Otherwise the `value` is `false`.

Semantics follow the model described in [P3780R0](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3780r0.html). The library keeps this separate trait for portability and because its internal optimizations rely on the "memcpy relocation" guarantee.

## Definition

The primary template is defined as:

```cpp
template <class T>
struct is_bitwise_trivially_relocatable
    : std::bool_constant<std::is_trivially_move_constructible_v<T> &&
                         std::is_trivially_destructible_v<T>> {};
```

Thus, by default, any type that is both trivially move constructible and trivially destructible is treated as bitwise trivially relocatable. Users may explicitly specialize the trait to `std::true_type` for additional types that meet the semantic requirement even if they are not both trivially move constructible and trivially destructible (e.g., certain pointer-like wrapper types). Users must not specialize it to `false` when the primary template would yield `true`.

In addition to the primary template, the implementation provides (positive) specializations for types that are known to satisfy the bitwise trivial relocation property:

- `std::unique_ptr<T, D>` when `D` is bitwise trivially relocatable
- `std::shared_ptr<T>`
- `std::weak_ptr<T>`
- *inplace-ptr&lt;T&gt;* when `T` is bitwise trivially relocatable (see [function template `make_proxy_inplace`](make_proxy_inplace.md))
- *allocated-ptr&lt;T, Alloc&gt;* (see [function template `allocate_proxy`](allocate_proxy.md))
- *strong-compact-ptr&lt;T, Alloc&gt;* (see [function template `allocate_proxy_shared`](allocate_proxy_shared.md))
- *weak-compact-ptr&lt;T, Alloc&gt;* (see [function template `allocate_proxy_shared`](allocate_proxy_shared.md))

These specializations reflect empirical knowledge of the representations of common "fancy pointer" types: relocating them with a raw byte copy preserves their invariants, and skipping destructor invocation of the source object has no observable effect beyond finalization already accounted for in the target representation.

## Notes

You may provide additional specializations of `is_bitwise_trivially_relocatable<T>` (in namespace `pro`) to opt in types you own. A correct specialization must ensure the type does not depend on its *address* remaining stable (self-pointers, intrusive container hooks, pointer provenance, etc.).

A positive specialization is a promise you must uphold. Violating the contract results in undefined behavior in any operation that uses the fast relocation path (e.g., certain `proxy` conversions or assignments).

## Example

```cpp
#include <type_traits>

#include <proxy/proxy.h>

struct Any : pro::facade_builder //
             ::build {};         // Requires trivial relocatability by default

struct A {
  int Val;
};

struct B {
  B() = default;
  B(B&&) noexcept {}

  int Val;
};

struct C {
  C() = default;
  C(B&&) noexcept {}

  int Val;
};

namespace pro {

template <>
struct is_bitwise_trivially_relocatable<C> : std::true_type {};

} // namespace pro

int main() {
  static_assert(pro::inplace_proxiable_target<A, Any>);
  static_assert(!pro::inplace_proxiable_target<B, Any>);
  static_assert(pro::inplace_proxiable_target<C, Any>);
}
```

## See Also

- [named requirements *ProFacade*](ProFacade.md)
- [`basic_facade_builder::support_relocation`](basic_facade_builder/support_relocation.md)
