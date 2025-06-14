# Function template `make_proxy`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v4`

The definitions of `make_proxy` make use of the following exposition-only function:

```cpp
template <facade F, class T, class... Args>
proxy<F> make-proxy-internal(Args&&... args) {
  if constexpr (inplace_proxiable_target<T, F>) {
    return make_proxy_inplace<F, T>(std::forward<Args>(args)...);
  } else {
    return allocate_proxy<F, T>(std::allocator<void>{}, std::forward<Args>(args)...);
  }
}
```

```cpp
// (1)
template <facade F, class T, class... Args>
proxy<F> make_proxy(Args&&... args);  // freestanding-deleted

// (2)
template <facade F, class T, class U, class... Args>
proxy<F> make_proxy(std::initializer_list<U> il, Args&&... args);  // freestanding-deleted

// (3)
template <facade F, class T>
proxy<F> make_proxy(T&& value);  // freestanding-deleted
```

`(1)` Equivalent to `return make-proxy-internal<F, T>(std::forward<Args>(args)...)`.

`(2)` Equivalent to `return make-proxy-internal<F, T>(il, std::forward<Args>(args)...)`.

`(3)` Equivalent to `return make-proxy-internal<F, std::decay_t<T>>(std::forward<T>(value))`.

*Since 3.3.0*: For `(1-3)`, if [`proxiable_target<std::decay_t<T>, F>`](proxiable_target.md) is `false`, the program is ill-formed and diagnostic messages are generated.

## Return Value

The constructed `proxy` object.

## Exceptions

Throws any exception thrown by allocation and the constructor of `T`.

## Example

```cpp
#include <iomanip>
#include <iostream>
#include <string>

#include <proxy/proxy.h>

struct Printable : pro::facade_builder //
                   ::add_convention<pro::operator_dispatch<"<<", true>,
                                    std::ostream&(std::ostream&) const> //
                   ::build {};

int main() {
  // From bool
  pro::proxy<Printable> p1 = pro::make_proxy<Printable>(true);
  // Prints "true"
  std::cout << std::boolalpha << *p1 << "\n";

  // From int
  pro::proxy<Printable> p2 = pro::make_proxy<Printable>(123);
  // Prints "123"
  std::cout << *p2 << "\n";

  // From double
  pro::proxy<Printable> p3 = pro::make_proxy<Printable>(3.1415926);
  // Prints "3.1415926000"
  std::cout << std::fixed << std::setprecision(10) << *p3 << "\n";

  // From const char*
  pro::proxy<Printable> p4 = pro::make_proxy<Printable>("lalala");
  // Prints "lalala"
  std::cout << *p4 << "\n";

  // From a in-place constructed string
  pro::proxy<Printable> p5 = pro::make_proxy<Printable, std::string>(5, 'x');
  // Prints "xxxxx"
  std::cout << *p5 << "\n";
}
```

## See Also

- [function template `make_proxy_inplace`](make_proxy_inplace.md)
- [function template `allocate_proxy`](allocate_proxy.md)
