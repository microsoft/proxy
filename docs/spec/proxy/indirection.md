# `proxy::operator->`<br />`proxy::operator*`

```cpp
// (1)
proxy_indirect_accessor<F>* operator->() noexcept;
const proxy_indirect_accessor<F>* operator->() const noexcept;

// (2)
proxy_indirect_accessor<F>& operator*() & noexcept;
const proxy_indirect_accessor<F>& operator*() const& noexcept;
proxy_indirect_accessor<F>&& operator*() && noexcept;
const proxy_indirect_accessor<F>&& operator*() const&& noexcept;
```

These operators access the accessors of the indirect conventions, as if dereferencing the contained value.

- `(1)` Returns a pointer to the `proxy_indirect_accessor<F>`.
- `(2)` Returns a reference to the `proxy_indirect_accessor<F>`.

The behavior is undefined if `*this` does not contain a value.

## Notes

These operators do not check whether the `proxy` contains a value. To check whether the `proxy` contains a value, call [`has_value()`](operator_bool.md) or use [operator ==](friend_operator_equality.md).

## Example

```cpp
#include <iostream>
#include <string>
#include <vector>

#include <proxy/proxy.h>

PRO_DEF_MEM_DISPATCH(MemSize, size);

struct BasicContainer
    : pro::facade_builder                                       //
      ::add_convention<MemSize, std::size_t() const & noexcept> //
      ::build {};

PRO_DEF_FREE_DISPATCH(FreeToString, std::to_string, ToString);

struct Stringable : pro::facade_builder                           //
                    ::add_convention<FreeToString, std::string()> //
                    ::build {};

int main() {
  std::vector<int> v(10);
  pro::proxy<BasicContainer> p0 = &v;
  std::cout << p0->size() << "\n";   // Prints "10"
  std::cout << (*p0).size() << "\n"; // Prints "10"

  pro::proxy<Stringable> p1 = pro::make_proxy<Stringable>(123);
  std::cout << ToString(*p1) << "\n"; // Prints "123"
}
```

## See Also

- [class template `proxy_indirect_accessor`](../proxy_indirect_accessor.md)
