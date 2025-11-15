# `proxy::emplace`

```cpp
// (1)
template <class P, class... Args>
P& emplace(Args&&... args)
    noexcept(std::is_nothrow_constructible_v<P, Args...> &&
        F::destructibility >= constraint_level::nothrow)
    requires(std::is_constructible_v<P, Args...> &&
        F::destructibility >= constraint_level::nontrivial);

// (2)
template <class P, class U, class... Args>
P& emplace(std::initializer_list<U> il, Args&&... args)
    noexcept(std::is_nothrow_constructible_v<
        P, std::initializer_list<U>&, Args...> &&
        F::destructibility >= constraint_level::nothrow)
    requires(std::is_constructible_v<P, std::initializer_list<U>&, Args...> &&
        F::destructibility >= constraint_level::nontrivial);
```

The `emplace` function templates change the contained value to an object of type `P` constructed from the arguments.

First, the current contained value (if any) is destroyed as if by calling [reset()](reset.md). Then:

- `(1)` Sets the contained value to an object of type `P`, direct-non-list-initialized with `std::forward<Args>(args)...`. Participates in overload resolution only if `P` is a pointer-like type eligible for `proxy` (see [*ProFacade* requirements](../ProFacade.md)).
- `(2)` Sets the contained value to an object of type `P`, direct-non-list-initialized with `il, std::forward<Args>(args)...`. Participates in overload resolution only if `P` is a pointer-like type eligible for `proxy`.

*Since 3.3.0*: For `(1-2)`, if [`proxiable<P, F>`](../proxiable.md) is `false`, the program is ill-formed and diagnostic messages are generated.

## Return Value

A reference to the newly created contained value.

## Exceptions

Throws any exception thrown by `P`'s constructor. If an exception is thrown, the previously contained value (if any) is destroyed, and `*this` does not contain a value.

## Comparison with Other Standard Polymorphic Wrappers

Similar to [`std::any::emplace`](https://en.cppreference.com/w/cpp/utility/any/emplace), `proxy` supports in-place construction for the contained value. However, unlike `std::any`, the overload of `proxy::emplace` that takes a value of type `std::in_place_type_t<T>` creates a value of type `T` rather than `std::decay_t<T>`. This makes the semantics of `proxy::emplace` simpler than those of [`std::any::emplace`](https://en.cppreference.com/w/cpp/utility/any/emplace).

## Example

```cpp
#include <iostream>
#include <memory>
#include <memory_resource>

#include <proxy/proxy.h>

struct AnyCopyable : pro::facade_builder                               //
                     ::support_copy<pro::constraint_level::nontrivial> //
                     ::build {};

struct Foo {
  ~Foo() { puts("Destroy Foo"); }

  int payload[10000];
};

int main() {
  static std::pmr::unsynchronized_pool_resource my_memory_pool;

  std::pmr::polymorphic_allocator<> alloc{&my_memory_pool};
  auto deleter = [alloc](auto* ptr) mutable { alloc.delete_object(ptr); };

  pro::proxy<AnyCopyable> p0;
  p0.emplace<std::shared_ptr<Foo>>(alloc.new_object<Foo>(), deleter);

  // `Foo` is not copied. Only the reference count is increased.
  pro::proxy<AnyCopyable> p1 = p0;
} // The destructor of `Foo` is called once when both `p0` and `p1` are
  // destroyed
```

## See Also

- [(constructor)](constructor.md)
- [`reset`](reset.md)
