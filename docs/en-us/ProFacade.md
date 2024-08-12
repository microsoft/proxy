# Named requirements: *ProFacade*

A type `F` meets the *ProFacade* requirements of a type `P` if `F` meets the [*ProBasicFacade* requirements](ProBasicFacade.md), and `P` meets the requirements defined by [`F::constraints`](proxiable_ptr_constraints.md), and the following expressions are well-formed and have the specified semantics.

| Expressions                    | Semantics                                                    |
| ------------------------------ | ------------------------------------------------------------ |
| `typename F::convention_types` | A [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type that contains any number of distinct types `Cs`. Each type `C` in `Cs` shall meet the [*ProConvention* requirements](ProConvention.md) of `P`. |
| `typename F::reflection_types` | A [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type that contains any number of distinct types `Rs`. Each type `R` in `Rs` shall meet the [*ProReflection* requirements](ProReflection.md) of `P`. |

## See Also

- [concept `proxiable`](proxiable.md)
- [*ProBasicFacade* requirements](ProBasicFacade.md)
