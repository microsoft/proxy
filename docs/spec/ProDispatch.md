# Named requirements: *ProDispatch*

A type `D` meets the *ProDispatch* requirements of types `T` and  `O` if `D` is a [trivial type](https://en.cppreference.com/w/cpp/named_req/TrivialType), `O` meets the [*ProOverload* requirelemts](ProOverload.md), and the following expressions are well-formed and have the specified semantics (let `R` be return type of `O`, `Args...` be the argument types of `O`. `args...` denotes values of type `Args...`, `v` denotes a value of type `T`, `cv` denotes a value of type `const T`).

| Definitions of `O`            | Expressions                                                  | Semantics                                                    |
| ----------------------------- | ------------------------------------------------------------ | ------------------------------------------------------------ |
| `R(Args...)`                  | [`INVOKE<R>`](https://en.cppreference.com/w/cpp/utility/functional)`(D{}, v, std::forward<Args>(args)...)` | Invokes dispatch type `D` with an lvalue reference of type `T` and `args...`, may throw. |
| `R(Args...) noexcept`         | [`INVOKE<R>`](https://en.cppreference.com/w/cpp/utility/functional)`(D{}, v, std::forward<Args>(args)...)` | Invokes dispatch type `D` with an lvalue reference of type `T` and `args...`, shall not throw. |
| `R(Args...) &`                | [`INVOKE<R>`](https://en.cppreference.com/w/cpp/utility/functional)`(D{}, v, std::forward<Args>(args)...)` | Invokes dispatch type `D` with an lvalue reference of type `T` and `args...`, may throw. |
| `R(Args...) & noexcept`       | [`INVOKE<R>`](https://en.cppreference.com/w/cpp/utility/functional)`(D{}, v, std::forward<Args>(args)...)` | Invokes dispatch type `D` with an lvalue reference of type `T` and `args...`, shall not throw. |
| `R(Args...) &&`               | [`INVOKE<R>`](https://en.cppreference.com/w/cpp/utility/functional)`(D{}, std::move(v), std::forward<Args>(args)...)` | Invokes dispatch type `D` with a rvalue reference of type `T` and `args...`, may throw. |
| `R(Args...) && noexcept`      | [`INVOKE<R>`](https://en.cppreference.com/w/cpp/utility/functional)`(D{}, std::move(v), std::forward<Args>(args)...)` | Invokes dispatch type `D` with a rvalue reference of type `T` and `args...`, shall not throw. |
| `R(Args...) const`            | [`INVOKE<R>`](https://en.cppreference.com/w/cpp/utility/functional)`(D{}, cv, std::forward<Args>(args)...)` | Invokes dispatch type `D` with a const reference of type `T` and `args...`, may throw. |
| `R(Args...) const noexcept`   | [`INVOKE<R>`](https://en.cppreference.com/w/cpp/utility/functional)`(D{}, cv, std::forward<Args>(args)...)` | Invokes dispatch type `D` with a const reference of type `T` and `args...`, shall not throw. |
| `R(Args...) cosnt&`           | [`INVOKE<R>`](https://en.cppreference.com/w/cpp/utility/functional)`(D{}, cv, std::forward<Args>(args)...)`, or<br />`d(nullptr, std::forward<Args>(args)...)` | Invokes dispatch type `D` with a const reference of type `T` and `args...`, may throw. |
| `R(Args...) const& noexcept`  | [`INVOKE<R>`](https://en.cppreference.com/w/cpp/utility/functional)`(D{}, cv, std::forward<Args>(args)...)` | Invokes dispatch type `D` with a const reference of type `T` and `args...`, shall not throw. |
| `R(Args...) const&&`          | [`INVOKE<R>`](https://en.cppreference.com/w/cpp/utility/functional)`(D{}, std::move(cv), std::forward<Args>(args)...)` | Invokes dispatch type `D` with a const rvalue reference of type `T` and `args...`, may throw. |
| `R(Args...) const&& noexcept` | [`INVOKE<R>`](https://en.cppreference.com/w/cpp/utility/functional)`(D{}, std::move(cv), std::forward<Args>(args)...)` | Invokes dispatch type `D` with a const rvalue reference of type `T` and `args...`, shall not throw. |

Or,

| Definitions of `O`         | Expressions                                                  | Semantics                                                    |
| -------------------------- | ------------------------------------------------------------ | ------------------------------------------------------------ |
| `R(Args...)` *cv ref noex* | [`INVOKE<R>`](https://en.cppreference.com/w/cpp/utility/functional)`(D{}, nullptr, std::forward<Args>(args)...)` | Invokes the dispatch type `D` with `nullptr` and `args...`, may or may not throw depending on `noex`. |

## See Also

- [*ProConvention* requirements](ProConvention.md)
