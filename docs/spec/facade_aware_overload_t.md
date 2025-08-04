# Class template `facade_aware_overload_t`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v4`  
> Since: 3.2.1

```cpp
template <template <class> class O>
struct facade_aware_overload_t { facade_aware_overload_t() = delete; };
```

Class template `facade_aware_overload_t<O>` specifies a facade-aware overload template `O`. It is useful when modeling an [overload](ProOverload.md) type of a [facade](facade.md) type that recursively depends on the facade type itself.

## Notes

`facade_aware_overload_t` can be used to define a convention in a base facade type, and is portable to the definition of another facade type via [`basic_facade_builder::add_facade`](basic_facade_builder/add_facade.md). It can also effectively avoid a facade type being implicitly instantiated when it is incomplete.

## Example

```cpp
#include <iostream>

#include <proxy/proxy.h>

template <class F>
using BinaryOverload =
    pro::proxy<F>(const pro::proxy_indirect_accessor<F>& rhs) const;

template <class T, pro::facade F>
pro::proxy<F> operator+(const T& value,
                        const pro::proxy_indirect_accessor<F>& rhs)
  requires(!std::is_same_v<T, pro::proxy_indirect_accessor<F>>)
{
  return pro::make_proxy<F, T>(value + proxy_cast<const T&>(rhs));
}

struct Addable
    : pro::facade_builder              //
      ::add_skill<pro::skills::rtti>   //
      ::add_skill<pro::skills::format> //
      ::add_convention<pro::operator_dispatch<"+">,
                       pro::facade_aware_overload_t<BinaryOverload>> //
      ::build {};

int main() {
  pro::proxy<Addable> p1 = pro::make_proxy<Addable>(1);
  pro::proxy<Addable> p2 = pro::make_proxy<Addable>(2);
  pro::proxy<Addable> p3 = *p1 + *p2;
  std::cout << std::format("{}\n", *p3); // Prints "3"
}
```

## See Also

- [*ProOverload* requirements](ProOverload.md)
- [alias template `skills::as_view`](skills_as_view.md)
