# Alias template `format`<br />Alias template `wformat`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v4::skills`  
> Since: 4.0.0

```cpp
template <class FB>
using format = /* see below */;

template <class FB>
using wformat = /* see below */;
```

The alias templates `format` and `wformat` modify a specialization of [`basic_facade_builder`](basic_facade_builder/README.md) to allow formatting via the [standard formatting functions](https://en.cppreference.com/w/cpp/utility/format). Specializations of `std::formatter<proxy_indirect_accessor<F>, char>` and `std::formatter<proxy_indirect_accessor<F>, wchar_t>` will be enabled for the 2 skills, respectively, where `F` is a built [facade](facade.md) type.

`format` and `wformat` also add constraints to a built facade type `F`, requiring a contained value of `proxy<F>` be *formattable*. Formally, let `p` be a contained value of `proxy<F>`, `CharT` be `char` (for `format`) or `wchar_t` (for `wformat`), `T` be `std::decay_t<decltype(*std::as_const(p))>`, `std::formatter<T, CharT>` shall be an enabled specialization of `std::formatter`.

## Example

```cpp
#include <format>
#include <iostream>

#include <proxy/proxy.h>

struct Formattable : pro::facade_builder              //
                     ::add_skill<pro::skills::format> //
                     ::build {};

int main() {
  pro::proxy<Formattable> p = pro::make_proxy<Formattable>(123);
  std::cout << std::format("{}", *p) << "\n";     // Prints "123"
  std::cout << std::format("{:*<6}", *p) << "\n"; // Prints "123***"
}
```

## See Also

- [`basic_facade_builder::add_skill`](basic_facade_builder/add_skill.md)
- [alias template `skills::fmt_format`](skills_fmt_format.md)
