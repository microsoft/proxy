# Named requirements: *ProOverload*

A type `O` meets the *ProOverload* requirements if it matches one of the following definitions, where `R` is the *return type*, `Args...` are the *argument types*.

| Definitions of `O`            |
| ----------------------------- |
| `R(Args...)`                  |
| `R(Args...) noexcept`         |
| `R(Args...) &`                |
| `R(Args...) & noexcept`       |
| `R(Args...) &&`               |
| `R(Args...) && noexcept`      |
| `R(Args...) const`            |
| `R(Args...) const noexcept`   |
| `R(Args...) cosnt&`           |
| `R(Args...) const& noexcept`  |
| `R(Args...) const&&`          |
| `R(Args...) const&& noexcept` |

## See Also

- [*ProConvention* requirements](ProConvention.md)
- [class template `std::move_only_function`](https://en.cppreference.com/w/cpp/utility/functional/move_only_function)
