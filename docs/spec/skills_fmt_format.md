# Alias template `fmt_format`<br />Alias template `fmt_wformat`

> Header: `proxy_fmt.h`  
> Module: None  
> Namespace: `pro::inline v4::skills`  
> Since: 4.0.0

```cpp
template <class FB>
using fmt_format = /* see below */;

template <class FB>
using fmt_wformat = /* see below */;
```

The alias templates `fmt_format` and `fmt_wformat` modify a specialization of [`basic_facade_builder`](basic_facade_builder/README.md) to allow formatting via [the {fmt} library](https://github.com/fmtlib/fmt). Specializations of `fmt::formatter<proxy_indirect_accessor<F>, char>` and `fmt::formatter<proxy_indirect_accessor<F>, wchar_t>` will be enabled for the 2 skills, respectively, where `F` is a built [facade](facade.md) type.

`fmt_format` and `fmt_wformat` also add constraints to a built facade type `F`, requiring a contained value of `proxy<F>` be *formattable*. Formally, let `p` be a contained value of `proxy<F>`, `CharT` be `char` (for `format`) or `wchar_t` (for `wformat`), `T` be `std::decay_t<decltype(*std::as_const(p))>`, `fmt::formatter<T, CharT>` shall be an enabled specialization of `fmt::formatter`.

## Notes

This facility requires [the {fmt} library](https://github.com/fmtlib/fmt) to be properly installed and included before including `proxy_fmt.h`.

- The minimum required version of {fmt} is 6.1.0.
- If you are using {fmt} version 8.0.0 or newer, you must also include the header `fmt/xchar.h`.
- When using modules, ensure that the macro `FMT_VERSION` is predefined.

## Example

```cpp
#include <iostream>

#include <fmt/format.h>
#include <fmt/xchar.h>
#include <proxy/proxy.h>
#include <proxy/proxy_fmt.h>

struct Formattable : pro::facade_builder                  //
                     ::add_skill<pro::skills::fmt_format> //
                     ::build {};

int main() {
  pro::proxy<Formattable> p = pro::make_proxy<Formattable>(3.14159);
  std::cout << fmt::format("*p = {:.2f}\n", *p); // Prints "*p = 3.14"
}
```

## See Also

- [`basic_facade_builder::add_skill`](basic_facade_builder/add_skill.md)
- [alias template `skills::format`](skills_format.md)
