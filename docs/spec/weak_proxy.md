# Alias template `weak_proxy`<br />Class template `weak_facade`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v4`  
> Since: 3.3.0

```cpp
template <facade F>
struct weak_facade;

template <facade F>
using weak_proxy = proxy<weak_facade<F>>;
```

Class template `weak_facade` is a [facade](facade.md) type for weak pointers (e.g., [`std::weak_ptr`](https://en.cppreference.com/w/cpp/memory/weak_ptr)) potentially converted from a `proxy` object.

## Member Types of `weak_facade`

| Name               | Description                                                  |
| ------------------ | ------------------------------------------------------------ |
| `convention_types` | A [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type that contains one convention type `C`, where`C::is_direct` is `false`, `C::dispatch_type` is of member function `lock` with accessibility, and `C::overload_types` is a [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type that contains type `proxy<F>() const noexcept`. |
| `reflection_types` | A [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type that contains no types. |

## Member Constants of `weak_facade`

| Name                               | Description                    |
| ---------------------------------- | ------------------------------ |
| `constraints` [static] [constexpr] | Equivalent to `F::constraints` |

## Example

```cpp
#include <iostream>

#include <proxy/proxy.h>

struct Formattable : pro::facade_builder            //
                     ::support<pro::skills::format> //
                     ::build {};

int main() {
  std::shared_ptr<int> val = std::make_shared<int>(123);
  pro::weak_proxy<Formattable> wp = std::weak_ptr{val};
  pro::proxy<Formattable> p = wp.lock();
  std::cout << std::boolalpha << p.has_value() << "\n"; // Prints "true"
  std::cout << std::format("{}\n", *p);                 // Prints "123"

  p.reset();
  val.reset();
  p = wp.lock();
  std::cout << p.has_value() << "\n"; // Prints "false"
}
```

## See Also

- [alias template `skills::as_weak`](skills_as_weak.md)
- [function template `make_proxy_shared`](make_proxy_shared.md)
