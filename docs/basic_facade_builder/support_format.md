# `basic_facade_builder::support_format`<br />`basic_facade_builder::support_wformat`

```cpp
using support_format = basic_facade_builder</* see below */>;

using support_wformat = basic_facade_builder</* see below */>;
```

The member types `support_format` and `support_wformat` of `basic_facade_builder<Cs, Rs, C>` add necessary convention and reflection types to the template parameters, enabling specializations of [`std::formatter<proxy_indirect_accessor<F>, CharT>`](../formatter_proxy_indirect_accessor.md) where `F` is a [facade](../facade.md) type built from `basic_facade_builder`, `CharT` is `char` (if `support_format` is specified) or `wchar_t` (if `support_wformat` is specified).

`support_format` and `support_wformat` also add constraints to a facade type `F` built from `basic_facade_builder`, requiring a contained value of `proxy<F>` be *formattable*. Formally, let `p` be a contained value of `proxy<F>`, `CharT` be `char` (if `support_format` is specified) or `wchar_t` (if `support_wformat` is specified), `T` be `std::decay_t<decltype(*std::as_const(p))>`, `std::formatter<T, CharT>` shall be an enabled specialization of `std::formatter`.

## Example

```cpp
#include <format>
#include <iostream>

#include "proxy.h"

struct Formattable : pro::facade_builder
    ::support_format
    ::build {};

int main() {
  pro::proxy<Formattable> p = pro::make_proxy<Formattable>(123);
  std::cout << std::format("{}", *p) << "\n";  // Prints: "123"
  std::cout << std::format("{:*<6}", *p) << "\n";  // Prints: "123***"
}
```

## See Also

- [class template `std::formatter<proxy_indirect_accessor>`](../formatter_proxy_indirect_accessor.md)
