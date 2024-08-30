# Class `proxiable_ptr_constraints`

```cpp
struct proxiable_ptr_constraints;
```

## Member objects

| Name              | Type                                      |
| ----------------- | ----------------------------------------- |
| `max_size`        | `std::size_t`                             |
| `max_align`       | `std::size_t`                             |
| `copyability`     | [`constraint_level`](constraint_level.md) |
| `relocatability`  | [`constraint_level`](constraint_level.md) |
| `destructibility` | [`constraint_level`](constraint_level.md) |

## Constraints to a Pointer Type

Given a pointer type `P` and a value `C` of type `proxiable_ptr_constraints`, `P` meets the constraints of `C` if all the following expressions are `true`.

**Layout constraints**: `sizeof(P) <= C::max_size && alignof(P) <= C::max_align`.

**Copyability constraints**:

| Possible value of `C::copyability` | Expressions                                                  |
| ---------------------------------- | ------------------------------------------------------------ |
| `constraint_level::none`           | `true`                                                       |
| `constraint_level::nontrivial`     | `std::is_copy_constructible_v<P>`                            |
| `constraint_level::nothrow`        | `std::is_nothrow_copy_constructible_v<P>`                    |
| `constraint_level::trivial`        | `std::is_trivially_copy_constructible_v<P> && std::is_trivially_destructible_v<P>` |

**Relocatability constraints**:

| Possible value of `C::relocatability` | Expressions                                                  |
| ------------------------------------- | ------------------------------------------------------------ |
| `constraint_level::none`              | `true`                                                       |
| `constraint_level::nontrivial`        | `std::is_move_constructible_v<P> && std::is_destructible_v<P>` |
| `constraint_level::nothrow`           | `std::is_nothrow_move_constructible_v<P> && std::is_nothrow_destructible_v<P>` |
| `constraint_level::trivial`           | `std::is_trivially_move_constructible_v<P> && std::is_trivially_destructible_v<P>` |

**Destructibility constraints**:

| Possible value of `C::relocatability` | Expressions                           |
| ------------------------------------- | ------------------------------------- |
| `constraint_level::none`              | `true`                                |
| `constraint_level::nontrivial`        | `std::is_destructible_v<P>`           |
| `constraint_level::nothrow`           | `std::is_nothrow_destructible_v<P>`   |
| `constraint_level::trivial`           | `std::is_trivially_destructible_v<P>` |

## See Also

- [concept `facade`](facade.md)
