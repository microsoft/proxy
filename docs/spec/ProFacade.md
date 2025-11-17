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

*Since 4.0.2*: `P` shall be a pointer-like type eligible for `proxy`. A type `P` is eligible if `P` is not a specialization of `proxy` and the following condition is satisfied:

```cpp
(requires { *std::declval<P&>(); } || requires { typename P::element_type; }) &&
requires { typename std::pointer_traits<P>::element_type; }
```

In other words, `P` either supports dereferencing or provides an `element_type`, and `std::pointer_traits<P>` yields a valid `element_type`.

## Notes

Relocatability is defined as *move-construct an object and then destroy the original instance*. Specifically, the value of `F::relocatability` maps to the following requirements on `P`:

| Value                          | Requirement on `P`                                           |
| ------------------------------ | ------------------------------------------------------------ |
| `constraint_level::none`       | None                                                         |
| `constraint_level::nontrivial` | `(std::is_move_constructible_v<P> && std::is_destructible_v<P>) || `[`is_bitwise_trivially_relocatable_v<P>`](is_bitwise_trivially_relocatable.md) |
| `constraint_level::nothrow`    | `(std::is_nothrow_move_constructible_v<P> && std::is_nothrow_destructible_v<P>) || `[`is_bitwise_trivially_relocatable_v<P>`](is_bitwise_trivially_relocatable.md) |
| `constraint_level::trivial`    | [`is_bitwise_trivially_relocatable_v<P>`](is_bitwise_trivially_relocatable.md) |

## See Also

- [concept `proxiable`](proxiable.md)
- [*ProBasicFacade* requirements](ProBasicFacade.md)
