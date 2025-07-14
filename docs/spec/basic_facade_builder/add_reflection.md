# `basic_facade_builder::add_reflection`<br />`basic_facade_builder::add_direct_reflection`<br />`basic_facade_builder::add_indirect_reflection`

```cpp
template <class R>
using add_reflection = add_indirect_reflection<R>;

template <class R>
using add_indirect_reflection = basic_facade_builder</* see below */>;

template <class R>
using add_direct_reflection = basic_facade_builder</* see below */>;
```

The alias templates `add_reflection`, `add_indirect_reflection` and `add_direct_reflection` of `basic_facade_builder<Cs, Rs, MaxSize, MaxAlign, Copyability, Relocatability, Destructibility>` add reflection types to the template parameters. Specifically,

- `add_reflection` is equivalent to `add_indirect_reflection`.
- `add_indirect_reflection` merges an implementation-defined reflection type `Refl` into `Rs`, where:
  - `Refl::is_direct` is `false`.
  - `typename Refl::reflector_type` is `R`.
  - `typename Refl::template accessor<F>` is `typename R::template accessor<proxy_indirect_accessor<F>, R>` if applicable.
- `add_direct_reflection` merges an implementation-defined reflection type `Refl` into `Rs`, where:
  - `Refl::is_direct` is `true`.
  - `typename Refl::reflector_type` is `R`.
  - `typename Refl::template accessor<F>` is `typename R::template accessor<proxy<F>, R>` if applicable.

When `Rs` already contains `Refl`, the template parameters shall not change.

## Notes

Adding duplicate reflection types is well-defined, whether done directly via `add_reflection`, `add_indirect_reflection`, `add_direct_reflection`, or indirectly via [`add_facade`](add_facade.md). This process does not affect the behavior of [`build`](build.md) at either compile-time or runtime.

## Example

```cpp
#include <array>
#include <iostream>

#include <proxy/proxy.h>

struct LayoutReflector {
public:
  template <class T>
  constexpr explicit LayoutReflector(std::in_place_type_t<T>)
      : Size(sizeof(T)), Align(alignof(T)) {}

  template <class P, class R>
  struct accessor {
    friend std::size_t SizeOf(const P& self) noexcept {
      const LayoutReflector& refl = pro::proxy_reflect<R>(self);
      return refl.Size;
    }

    friend std::size_t AlignOf(const P& self) noexcept {
      const LayoutReflector& refl = pro::proxy_reflect<R>(self);
      return refl.Align;
    }
  };

  std::size_t Size, Align;
};

struct LayoutAware : pro::facade_builder                        //
                     ::add_direct_reflection<LayoutReflector>   //
                     ::add_indirect_reflection<LayoutReflector> //
                     ::build {};

int main() {
  int a = 123;
  pro::proxy<LayoutAware> p = &a;
  std::cout << SizeOf(p) << "\n";   // Prints sizeof(raw pointer)
  std::cout << AlignOf(p) << "\n";  // Prints alignof(raw pointer)
  std::cout << SizeOf(*p) << "\n";  // Prints sizeof(int)
  std::cout << AlignOf(*p) << "\n"; // Prints alignof(int)

  p = pro::make_proxy<LayoutAware>(123); // SBO enabled
  std::cout << SizeOf(p) << "\n";        // Prints sizeof(int)
  std::cout << AlignOf(p) << "\n";       // Prints alignof(int)
  std::cout << SizeOf(*p) << "\n";       // Prints sizeof(int)
  std::cout << AlignOf(*p) << "\n";      // Prints alignof(int)

  p = pro::make_proxy<LayoutAware, std::array<char, 100>>(); // SBO disabled
  std::cout << SizeOf(p) << "\n";   // Prints sizeof(raw pointer)
  std::cout << AlignOf(p) << "\n";  // Prints alignof(raw pointer)
  std::cout << SizeOf(*p) << "\n";  // Prints "100"
  std::cout << AlignOf(*p) << "\n"; // Prints "1"
}
```

## See Also

- [named requirements: *ProReflection*](../ProReflection.md)
