# Named requirements: *ProBasicFacade*

A type `F` meets the *ProBasicFacade* requirements if the following expressions are well-formed and have the specified semantics.

| Expressions                    | Semantics                                                    |
| ------------------------------ | ------------------------------------------------------------ |
| `typename F::convention_types` | A [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type that contains any number of distinct types `Cs`. Each type `C` in `Cs` shall meet the [*ProBasicConvention* requirements](ProBasicConvention.md). |
| `typename F::reflection_types` | A [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type that contains any number of distinct types `Rs`. Each type `R` in `Rs` shall define reflection data structure. |
| `F::max_size`                  | A [core constant expression](https://en.cppreference.com/w/cpp/language/constant_expression) of type `std::size_t` that defines the maximum size of a pointer type. Shall be greater than `0` and a multiple of `F::max_align`. |
| `F::max_align`                 | A [core constant expression](https://en.cppreference.com/w/cpp/language/constant_expression) of type `std::size_t` that defines the maximum alignment of a pointer type. Shall be a power of `2`. |
| `F::copyability`               | A [core constant expression](https://en.cppreference.com/w/cpp/language/constant_expression) of type [`constraint_level`](constraint_level.md) that defines the required copyability of a pointer type. |
| `F::relocatability`            | A [core constant expression](https://en.cppreference.com/w/cpp/language/constant_expression) of type [`constraint_level`](constraint_level.md) that defines the required relocatability of a pointer type. |
| `F::destructibility`           | A [core constant expression](https://en.cppreference.com/w/cpp/language/constant_expression) of type [`constraint_level`](constraint_level.md) that defines the required destructibility of a pointer type. |

Each of `F::copyability`, `F::relocatability`, and `F::destructibility` shall be exactly one of the four enumerators of `constraint_level` (`none`, `nontrivial`, `nothrow`, `trivial`).

## See Also

- [concept `facade`](facade.md)
