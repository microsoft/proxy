# Named requirements: *ProConvention*

A type `C` meets the *ProConvention* requirements of a type `P` if `C` meets the [*ProBasicConvention* requirements](ProBasicConvention.md), and the following expressions are well-formed and have the specified semantics.

| Expressions                  | Semantics                                                    |
| ---------------------------- | ------------------------------------------------------------ |
| `typename C::overload_types` | A [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type that contains one or more distinct types `Os`. Each type `O` in `Os` shall meet the [*ProOverload* requirements](ProOverload.md), and<br />- when `C::is_direct` is `true`, `typename C::dispatch_type` shall meet the [*ProDispatch* requirements](ProDispatch.md) of `P` and `O`, <br />- or otherwise, when `C::is_direct` is `false`, let `QP` be a qualified reference type of `P` with the *cv ref* qualifiers defined by `O` (`QP` is an lvalue reference type if `O` does not define a *ref* qualifier), `qp` be a value of `QP`, `*std::forward<QP>(qp)` shall be well-formed, and `typename C::dispatch_type` shall meet the [*ProDispatch* requirements](ProDispatch.md) of `decltype(*std::forward<QP>(qp))` and `O`. |

## See Also

- [*ProFacade* requirements](ProFacade.md)
