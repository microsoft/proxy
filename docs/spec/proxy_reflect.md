# Function template `proxy_reflect`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v4`

```cpp
// (1)
template <class R, facade F>
const R& proxy_reflect(const proxy_indirect_accessor<F>& p) noexcept;

// (2)
template <class R, facade F>
const R& proxy_reflect(const proxy<F>& p) noexcept;
```

Acquires reflection information of a contained type.

- `(1)` Let `P` be the contained type of the `proxy` object associated to `p`. Returns a `const` reference of `R` direct-non-list-initialized with [`std::in_place_type<typename std::pointer_traits<P>::element_type>`](https://en.cppreference.com/w/cpp/utility/in_place).
- `(2)` Let `P` be the contained type of `p`. Returns a `const` reference of `R` direct-non-list-initialized with [`std::in_place_type<P>`](https://en.cppreference.com/w/cpp/utility/in_place). The behavior is undefined if `p` does not contain a value.

The reference obtained from `proxy_reflect()` may be invalidated if `p` is subsequently modified.

## Notes

This function is useful when only metadata deduced from a type is needed. While [`proxy_invoke`](proxy_invoke.md) can also retrieve type metadata, `proxy_reflect` can generate more efficient code in this context.

## Example

```cpp
#include <iostream>
#include <memory>
#include <type_traits>

#include <proxy/proxy.h>

class CopyabilityReflector {
public:
  template <class T>
  constexpr explicit CopyabilityReflector(std::in_place_type_t<T>)
      : copyable_(std::is_copy_constructible_v<T>) {}

  template <class P, class R>
  struct accessor {
    bool IsCopyable() const noexcept {
      const CopyabilityReflector& self =
          pro::proxy_reflect<R>(static_cast<const P&>(*this));
      return self.copyable_;
    }
  };

private:
  bool copyable_;
};

struct CopyabilityAware : pro::facade_builder                           //
                          ::add_direct_reflection<CopyabilityReflector> //
                          ::build {};

int main() {
  pro::proxy<CopyabilityAware> p1 = std::make_unique<int>();
  std::cout << std::boolalpha << p1.IsCopyable() << "\n"; // Prints "false"

  pro::proxy<CopyabilityAware> p2 = std::make_shared<int>();
  std::cout << p2.IsCopyable() << "\n"; // Prints "true"
}
```

## See Also

- [alias template `basic_facade_builder::add_reflection`](basic_facade_builder/add_reflection.md)
- [named requirements *ProReflection*](ProReflection.md)
