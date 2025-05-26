# Named requirements: *ProBasicConvention*

A type `C` meets the *ProBasicConvention* requirements if the following expressions are well-formed and have the specified semantics.

| Expressions                  | Semantics                                                    |
| ---------------------------- | ------------------------------------------------------------ |
| `C::is_direct`               | A [core constant expression](https://en.cppreference.com/w/cpp/language/constant_expression) of type `bool`, specifying whether the convention applies to a pointer type itself (`true`), or the element type of a pointer type (`false`). |
| `typename C::dispatch_type`  | A [trivial type](https://en.cppreference.com/w/cpp/named_req/TrivialType) that defines how the calls are forwarded to the concrete types. |
| `typename C::overload_types` | A [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type of one or more distinct types `Os`. Each type `O` in `Os` shall meet the [*ProOverload* requirements](ProOverload.md). |

## See Also

- [*ProBasicFacade* requirements](ProBasicFacade.md)
- [*ProConvention* requirements](ProConvention.md)
