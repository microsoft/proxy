# `basic_facade_builder::add_facade`

```cpp
template <facade F, bool WithUpwardConversion = false>
using add_facade = basic_facade_builder</* see below */>;
```

The alias template `add_facade` of `basic_facade_builder<Cs, Rs, C>` adds a [facade](../facade.md) type into the template parameters. It merges `typename F::convention_types` into `Cs`, `typename F::reflection_types` into `Rs`, and `F::constraints` into `C`. Optionally, it adds a convention for implicit upward conversion into `Cs` when `WithUpwardConversion` is `true`.

## Notes

Adding a facade type that contains duplicated convention or reflection types already defined in `Cs` or `Rs` is well-defined and does not have side effects on [`build`](build.md) at either compile-time or runtime. By default, `WithUpwardConversion` is `false`, which guarantees minimal binary size in code generation. However, upward conversion is helpful when an API requires backward compatibility. Users can opt-in to this feature by specifying `true` as the second parameter of `add_facade`, at the cost of potentially a slightly larger binary size.

## Example

```cpp
#include <iostream>
#include <unordered_map>

#include "proxy.h"

PRO_DEF_MEM_DISPATCH(MemSize, size);
PRO_DEF_MEM_DISPATCH(MemAt, at);
PRO_DEF_MEM_DISPATCH(MemEmplace, emplace);

struct Copyable : pro::facade_builder
    ::support_copy<pro::constraint_level::nontrivial>
    ::build {};

struct BasicContainer : pro::facade_builder
    ::add_convention<MemSize, std::size_t() const noexcept>
    ::build {};

struct StringDictionary : pro::facade_builder
    ::add_facade<BasicContainer>
    ::add_facade<Copyable>
    ::add_convention<MemAt, std::string(std::size_t key) const>
    ::build {};

struct MutableStringDictionary : pro::facade_builder
    ::add_facade<StringDictionary, true>
    ::add_convention<MemEmplace, void(std::size_t key, std::string value)>
    ::build {};

int main() {
  pro::proxy<MutableStringDictionary> p1 =
      pro::make_proxy<MutableStringDictionary, std::unordered_map<std::size_t, std::string>>();
  std::cout << p1->size() << "\n";  // Prints "0"
  try {
    std::cout << p1->at(123) << "\n";  // No output because the expression throws
  } catch (const std::out_of_range& e) {
    std::cerr << e.what() << "\n";  // Prints error message
  }
  p1->emplace(123, "lalala");
  auto p2 = p1;  // Performs a deep copy
  p2->emplace(456, "trivial");
  pro::proxy<StringDictionary> p3 = std::move(p2);  // Performs an upward conversion from an rvalue reference
  std::cout << p1->size() << "\n";  // Prints "1"
  std::cout << p1->at(123) << "\n";  // Prints "lalala"
  std::cout << std::boolalpha << p2.has_value() << "\n";  // Prints "false" because it is moved
  std::cout << p3->size() << "\n";  // Prints "2"
  std::cout << p3->at(123) << "\n";  // Prints "lalala"
  std::cout << p3->at(456) << "\n";  // Prints "trivial"
}
```

## See Also

- [`build`](build.md)
