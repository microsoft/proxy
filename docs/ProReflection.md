# Named requirements: *ProReflection*

A type `R` meets the *ProReflection* requirements of a type `P` if `R` meets the [*ProBasicReflection* requirements](ProBasicReflection.md), and the following expressions are well-formed and have the specified semantics (let `T` be `P` when `R::is_direct` is `true`, or otherwise `typename std::pointer_traits<P>::element_type`).

| Expressions                                         | Semantics                                                    |
| --------------------------------------------------- | ------------------------------------------------------------ |
| `typename R::reflector_type{std::in_place_type<T>}` | A [core constant expression](https://en.cppreference.com/w/cpp/language/constant_expression) that constructs a value of type `typename R::reflector_type`, reflecting implementation-defined metadata of type `T`. |

## See Also

[*ProFacade* requirements](ProFacade.md)
