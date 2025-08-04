# Function template `make_proxy_shared`

> Header: `proxy.h`  
> Module: `proxy`  
> Namespace: `pro::inline v4`  
> Since: 3.3.0

```cpp
// (1)
template <facade F, class T, class... Args>
proxy<F> make_proxy_shared(Args&&... args);  // freestanding-deleted

// (2)
template <facade F, class T, class U, class... Args>
proxy<F> make_proxy_shared(std::initializer_list<U> il, Args&&... args);  // freestanding-deleted

// (3)
template <facade F, class T>
proxy<F> make_proxy_shared(T&& value);  // freestanding-deleted
```

`(1)` Equivalent to `return allocate_proxy_shared<F, T>(std::allocator<void>{}, std::forward<Args>(args)...)`.

`(2)` Equivalent to `return allocate_proxy_shared<F, T>(std::allocator<void>{}, il, std::forward<Args>(args)...)`.

`(3)` Equivalent to `return allocate_proxy_shared<F, std::decay_t<T>>(std::allocator<void>{}, std::forward<T>(value))`.

*Since 3.3.0*: For `(1-3)`, if [`proxiable_target<std::decay_t<T>, F>`](proxiable_target.md) is `false`, the program is ill-formed and diagnostic messages are generated.

## Return Value

The constructed `proxy` object.

## Exceptions

Throws any exception thrown by allocation and the constructor of `T`.

## Example

```cpp
#include <iostream>

#include <proxy/proxy.h>

struct RttiAware : pro::facade_builder                            //
                   ::support_copy<pro::constraint_level::nothrow> //
                   ::add_skill<pro::skills::rtti>                 //
                   ::add_skill<pro::skills::as_weak>              //
                   ::build {};

int main() {
  pro::proxy<RttiAware> p1 = pro::make_proxy_shared<RttiAware>(123);
  pro::weak_proxy<RttiAware> p2 = p1;
  pro::proxy<RttiAware> p3 = p2.lock();
  std::cout << std::boolalpha << p3.has_value() << "\n"; // Prints "true"
  std::cout << proxy_cast<int>(*p3) << "\n";             // Prints "123"

  p3.reset();
  p1.reset();
  p3 = p2.lock();
  std::cout << std::boolalpha << p3.has_value() << "\n"; // Prints "false"
}
```

## See Also

- [function template `make_proxy`](make_proxy.md)
- [function template `allocate_proxy_shared`](allocate_proxy_shared.md)
