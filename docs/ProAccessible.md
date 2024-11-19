# Named requirements: *ProAccessible*

Given that `F` is a type meeting the [*ProBasicFacade* requirements](ProBasicFacade.md), a type `T` meets the *ProAccessible* requirements of types `F, Args...`, if the following expressions are well-formed and have the specified semantics.

| Expressions                                 | Semantics                                                    |
| ------------------------------------------- | ------------------------------------------------------------ |
| `typename T::template accessor<F, Args...>` | A type that provides accessibility to `proxy`. It shall be a *nothrow-default-constructible*, *trivially-copyable* type, and shall not be [final](https://en.cppreference.com/w/cpp/language/final). |

## See Also

- [class template `proxy`](proxy.md)
