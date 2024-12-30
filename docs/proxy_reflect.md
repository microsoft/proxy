# Function template `proxy_reflect`

```cpp
template <bool IsDirect, class R, class F>
const R& proxy_reflect(const proxy<F>& p) noexcept;
```

Let `P` be the type of the contained value of `p`. Retrieves a value of type `const R&` constructed from [`std::in_place_type<T>`](https://en.cppreference.com/w/cpp/utility/in_place), where `T` is `P` when `IsDirect` is `true`, or otherwise `T` is `typename std::pointer_traits<P>::element_type` when `IsDirect` is `false`. There shall be a reflection type `Refl` defined in `typename F::reflection_types` where `Refl::is_direct == IsDirect && std::is_same_v<typename Refl::reflection_type, R>` is `true`. The behavior is undefined if `p` does not contain a value.

The reference obtained from `proxy_reflect()` may be invalidated if `p` is subsequently modified.

## Notes

This function is useful when only metadata deduced from a type is needed. While [`proxy_invoke`](proxy_invoke.md) can also retrieve type metadata, `proxy_reflect` can generate more efficient code in this context.

## Example

```cpp
#include <iostream>
#include <memory>
#include <type_traits>

#include "proxy.h"

class CopyabilityReflector {
 public:
  template <class T>
  constexpr explicit CopyabilityReflector(std::in_place_type_t<T>)
      : copyable_(std::is_copy_constructible_v<T>) {}

  template <class F, bool IsDirect, class R>
  struct accessor {
    bool IsCopyable() const noexcept {
      const CopyabilityReflector& self = pro::proxy_reflect<IsDirect, R>(pro::access_proxy<F>(*this));
      return self.copyable_;
    }
  };

 private:
  bool copyable_;
};

struct CopyabilityAware : pro::facade_builder
    ::add_direct_reflection<CopyabilityReflector>
    ::build {};

int main() {
  pro::proxy<CopyabilityAware> p1 = std::make_unique<int>();
  std::cout << std::boolalpha << p1.IsCopyable() << "\n";  // Prints: "false"

  pro::proxy<CopyabilityAware> p2 = std::make_shared<int>();
  std::cout << p2.IsCopyable() << "\n";  // Prints: "true"
}
```

## See Also

- [alias template `basic_facade_builder::add_reflection`](basic_facade_builder/add_reflection.md)
- [named requirements *ProReflection*](ProReflection.md)
