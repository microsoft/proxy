# Named requirements: *ProReflection*

A type `R` meets the *ProReflection* requirements of a type `P` if the following expressions are well-formed and have the specified semantics.

| Expressions                | Semantics                                                    |
| -------------------------- | ------------------------------------------------------------ |
| `R{std::in_place_type<P>}` | A [core constant expression](https://en.cppreference.com/w/cpp/language/constant_expression) that constructs a value of type `R`, reflecting implementation-defined metadata of type `P`. |

## See Also

[*ProFacade* requirements](ProFacade.md)
