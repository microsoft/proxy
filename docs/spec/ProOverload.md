# Named requirements: *ProOverload*

A type `O` meets the *ProOverload* requirements if `substituted-overload<O, F>` matches one of the following definitions, where `F` is any type meeting the [*ProBasicFacade* requirements](ProBasicFacade.md), `R` is the *return type*, `Args...` are the *argument types*.

The exposition-only type `substituted-overload<O, F>` is `OT<F>` if `O` is a specialization of [`facade_aware_overload_t<OT>`](facade_aware_overload_t.md), or `O` otherwise.

| Definitions of `substituted-overload<O, F>` |
| ------------------------------------------- |
| `R(Args...)`                                |
| `R(Args...) noexcept`                       |
| `R(Args...) &`                              |
| `R(Args...) & noexcept`                     |
| `R(Args...) &&`                             |
| `R(Args...) && noexcept`                    |
| `R(Args...) const`                          |
| `R(Args...) const noexcept`                 |
| `R(Args...) const&`                         |
| `R(Args...) const& noexcept`                |
| `R(Args...) const&&`                        |
| `R(Args...) const&& noexcept`               |

## See Also

- [*ProConvention* requirements](ProConvention.md)
- [class template `std::move_only_function`](https://en.cppreference.com/w/cpp/utility/functional/move_only_function)
