# Named requirements: *ProBasicFacade*

A type `F` meets the *ProBasicFacade* requirements if the following expressions are well-formed and have the specified semantics.

| Expressions                    | Semantics                                                    |
| ------------------------------ | ------------------------------------------------------------ |
| `typename F::convention_types` | A [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type that contains any number of distinct types `Cs`. Each type `C` in `Cs` shall meet the [*ProBasicConvention* requirements](ProBasicConvention.md). |
| `typename F::reflection_types` | A [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type that contains any number of distinct types `Rs`. Each type `R` in `Rs` shall define reflection on pointer types. |
| `F::constraints`               | A [core constant expression](https://en.cppreference.com/w/cpp/language/constant_expression) of type [`proxiable_ptr_constraints`](proxiable_ptr_constraints.md) that defines constraints to pointer types. |

## See Also

- [concept `facade`](facade.md)
