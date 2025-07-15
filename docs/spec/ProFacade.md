# Named requirements: *ProFacade*

A type `F` meets the *ProFacade* requirements of a type `P` if `F` meets the [*ProBasicFacade* requirements](ProBasicFacade.md), and the following expressions are well-formed and have the specified semantics.

| Expressions                    | Semantics                                                    |
| ------------------------------ | ------------------------------------------------------------ |
| `typename F::convention_types` | A [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type that contains any number of distinct types `Cs`. Each type `C` in `Cs` shall meet the [*ProConvention* requirements](ProConvention.md) of `P`. |
| `typename F::reflection_types` | A [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type that contains any number of distinct types `Rs`. Each type `R` in `Rs` shall meet the [*ProReflection* requirements](ProReflection.md) of `P`. |
| `F::max_size`                  | A [core constant expression](https://en.cppreference.com/w/cpp/language/constant_expression) of type `std::size_t` that shall be greater than or equal to `sizeof(P)`. |
| `F::max_align`                 | A [core constant expression](https://en.cppreference.com/w/cpp/language/constant_expression) of type `std::size_t` that shall be greater than or equal to `alignof(P)`. |
| `F::copyability`               | A [core constant expression](https://en.cppreference.com/w/cpp/language/constant_expression) of type [`constraint_level`](constraint_level.md) that defines the required copyability of `P`. |
| `F::relocatability`            | A [core constant expression](https://en.cppreference.com/w/cpp/language/constant_expression) of type [`constraint_level`](constraint_level.md) that defines the required relocatability of `P`. |
| `F::destructibility`           | A [core constant expression](https://en.cppreference.com/w/cpp/language/constant_expression) of type [`constraint_level`](constraint_level.md) that defines the required destructibility of `P`. |

## Notes

Relocatability is defined as *move-construct an object and then destroy the original instance*. Specifically, the value of `F::relocatability` maps to the following requirements on `P`:

| Value                          | Requirement on `P`                                           |
| ------------------------------ | ------------------------------------------------------------ |
| `constraint_level::none`       | None                                                         |
| `constraint_level::nontrivial` | `std::is_move_constructible_v<P> && std::is_destructible_v<P>` |
| `constraint_level::nothrow`    | `std::is_nothrow_move_constructible_v<P> && std::is_nothrow_destructible_v<P>` |
| `constraint_level::trivial`    | *trivially relocatable* (see below)                          |

C++26 introduces the type trait `std::is_trivially_relocatable_v` ([P2786R13](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p2786r13.html)). The library supports C++20/23 and C++26 with a three-stage evaluation:

1. If `std::is_trivially_relocatable_v<P>` is available **and evaluates to `true`**, the requirement is satisfied.
2. Otherwise, a conservative fallback is applied: `std::is_trivially_move_constructible_v<P> && std::is_trivially_destructible_v<P>`.
3. If the fallback is `false`, the library consults an explicit curated allow-list of types that are known to be safely relocatable:
  - `std::unique_ptr<T, D>` when `D` is itself trivially relocatable
  - `std::shared_ptr<T>`
  - `std::weak_ptr<T>`

This sequence reduces the risk of false negatives while compiler support for C++26 is still being rolled out, without compromising safety.

## See Also

- [concept `proxiable`](proxiable.md)
- [*ProBasicFacade* requirements](ProBasicFacade.md)
