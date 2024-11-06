# Function template `proxy_reflect`

```cpp
template <class R, class F>
const R& proxy_reflect(const proxy<F>& p) noexcept;
```

Retrieves a value of type `R` constructed from [`std::in_place_type<P>`](https://en.cppreference.com/w/cpp/utility/in_place), where `P` is the type of the contained value of `p`. `R` is required to be defined in `typename F::reflection_types`. The behavior is undefined if `p` does not contain a value.

The reference obtained from `proxy_reflect()` may be invalidated if `p` is subsequently modified.

## Notes

This function is useful when only metadata deduced from a type is needed. While [`proxy_invoke`](proxy_invoke.md) can also retrieve type metadata, `proxy_reflect` can generate more efficient code in this context.

## Example

```cpp
#include <iostream>
#include <memory>
#include <type_traits>

#include "proxy.h"

struct TraitsRefl {
  template <class P>
  constexpr explicit TraitsRefl(std::in_place_type_t<P>)
      : Copyable(std::is_copy_constructible_v<P>) {}

  PRO_DEF_REFL_AS_MEM_ACCESSOR(ReflectTraits);

  const bool Copyable;
};

struct TestFacade : pro::facade_builder
    ::add_direct_reflection<TraitsRefl>
    ::build {};

int main() {
  pro::proxy<TestFacade> p1 = std::make_unique<int>();
  std::cout << std::boolalpha << p1.ReflectTraits().Copyable << "\n";  // Reflects with accessor, prints: "false"
  using R = std::tuple_element_t<0u, TestFacade::reflection_types>;
  std::cout << pro::proxy_reflect<R>(p1).Copyable << "\n";  // Reflects with proxy_reflect, also prints: "false"

  pro::proxy<TestFacade> p2 = std::make_shared<int>();
  std::cout << p2.ReflectTraits().Copyable << "\n";  // Reflects with accessor, prints: "true"
  std::cout << pro::proxy_reflect<R>(p2).Copyable << "\n";  // Reflects with proxy_reflect, also prints: "true"
}
```

## See Also

- [alias template `basic_facade_builder::add_reflection`](basic_facade_builder/add_reflection.md)
- [named requirements *ProReflection*](ProReflection.md)
