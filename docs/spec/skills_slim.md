# Alias template `slim`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v4::skills`  
> Since: 4.0.0

```cpp
template <class FB>
using slim = typename FB::template restrict_layout<sizeof(void*), alignof(void*)>;
```

The alias template `slim` modifies a specialization of [`basic_facade_builder`](basic_facade_builder/README.md) by restricting memory layout to single-pointer-size.

## Notes

Let `F` be a built [facade](facade.md) type.

- The contained pointer type of `proxy<F>` must itself fit into single-pointer-size.
   - Typical examples: `T*`, [`std::unique_ptr<T>`](https://en.cppreference.com/w/cpp/memory/unique_ptr.html) (empty deleter).
   - Counter-examples: [`std::shared_ptr<T>`](https://en.cppreference.com/w/cpp/memory/shared_ptr.html), [`std::unique_ptr<T, D>`](https://en.cppreference.com/w/cpp/memory/unique_ptr.html) with a stateful deleter.
- For best performance create `proxy<F>` objects via the library functions ( [`make_proxy`](make_proxy.md), [`allocate_proxy`](allocate_proxy.md), [`make_proxy_shared`](make_proxy_shared.md) and [`allocate_proxy_shared`](allocate_proxy_shared.md), etc.); they automatically honor the layout constraint.
- A `proxy<F>` object itself also stores bookkeeping metadata; its size is implementation-defined and can exceed single-pointer-size.

## Example

```cpp
#include <memory>

#include <proxy/proxy.h>

struct Default : pro::facade_builder::build {};

struct Slim : pro::facade_builder            //
              ::add_skill<pro::skills::slim> //
              ::build {};

int main() {
  static_assert(sizeof(pro::proxy<Default>) > sizeof(pro::proxy<Slim>));

  static_assert(pro::proxiable<int*, Default>);
  static_assert(pro::proxiable<std::unique_ptr<int>, Default>);
  static_assert(pro::proxiable<std::shared_ptr<int>, Default>);

  static_assert(pro::proxiable<int*, Slim>);
  static_assert(pro::proxiable<std::unique_ptr<int>, Slim>);

  // std::shared_ptr is too large for a slim facade
  static_assert(!pro::proxiable<std::shared_ptr<int>, Slim>);

  // pro::make_proxy_shared works with a slim facade
  pro::proxy<Slim> p = pro::make_proxy_shared<Slim>(123);
}
```

## See Also

- [`basic_facade_builder::add_skill`](basic_facade_builder/add_skill.md)
- [`basic_facade_builder::restrict_layout`](basic_facade_builder/restrict_layout.md)
