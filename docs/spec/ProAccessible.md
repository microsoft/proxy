# Named requirements: *ProAccessible*

A type `T` meets the *ProAccessible* requirements of types `Args...` if the following expressions are well-formed and have the specified semantics.

| Expressions                              | Semantics                                                    |
| ---------------------------------------- | ------------------------------------------------------------ |
| `typename T::template accessor<Args...>` | A type that provides accessibility to `proxy`. It shall be a *nothrow-default-constructible*, *trivially-copyable* type, and shall not be [final](https://en.cppreference.com/w/cpp/language/final). |

## See Also

- [class template `proxy`](proxy/README.md)
- [class template `proxy_indirect_accessor`](proxy_indirect_accessor.md)
