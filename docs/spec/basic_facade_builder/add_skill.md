# `basic_facade_builder::add_skill`

> Since: 4.0.0

```cpp
template <template <class> class Skill>
    requires(/* see below */)
using add_skill = Skill<basic_facade_builder>;
```

The alias template `add_skill` modifies template paratemeters with a custom skill. The expression inside `requires` is equivalent to `Skill<basic_facade_builder>` is a specialization of `basic_facade_builder`.

## Notes

The implementation of `Skill` can combine other member alias templates defined in `basic_facade_builder` as a skillset.

## Example

```cpp
#include <iostream>

#include <proxy/proxy.h>

template <class FB>
using SharedSlim = typename FB                                //
    ::template restrict_layout<sizeof(void*), alignof(void*)> //
    ::template support_copy<pro::constraint_level::nothrow>   //
    ::template add_skill<pro::skills::as_weak>;

struct SharedFormattable : pro::facade_builder              //
                           ::add_skill<SharedSlim>          //
                           ::add_skill<pro::skills::format> //
                           ::build {};

int main() {
  // Raw pointer does not have shared semantics
  static_assert(!pro::proxiable<int*, SharedFormattable>);

  // The implementation of std::shared_ptr is usually larger than sizeof(void*)
  static_assert(!pro::proxiable<std::shared_ptr<int>, SharedFormattable>);

  // The built-in shared pointer is guaranteed to be "slim"
  pro::proxy<SharedFormattable> p1 =
      pro::make_proxy_shared<SharedFormattable>(123);
  pro::weak_proxy<SharedFormattable> p2 = p1;
  pro::proxy<SharedFormattable> p3 = p2.lock();
  std::cout << std::format("{}\n", *p3); // Prints "123"
}
```

## See Also

- [`skills::as_view`](../skills_as_view.md)
- [`skills::format`](../skills_format.md)
- [`skills::rtti` ](../skills_rtti/README.md)
