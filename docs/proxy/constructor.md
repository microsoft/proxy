# `proxy<F>::proxy`

```cpp
// (1)
proxy() noexcept = default;
proxy(std::nullptr_t) noexcept;

// (2)
proxy(const proxy&) noexcept requires(F::constraints.copyability ==
    constraint_level::trivial) = default;
proxy(const proxy& rhs)
    noexcept(F::constraints.copyability == constraint_level::nothrow)
    requires(F::constraints.copyability == constraint_level::nontrivial ||
        F::constraints.copyability == constraint_level::nothrow);

// (3)
proxy(proxy&& rhs)
    noexcept(F::constraints.relocatability == constraint_level::nothrow)
    requires(F::constraints.relocatability >= constraint_level::nontrivial &&
        F::constraints.copyability != constraint_level::trivial);

// (4)
template <class P>
proxy(P&& ptr) noexcept(std::is_nothrow_constructible_v<std::decay_t<P>, P>)
    requires(proxiable<std::decay_t<P>, F> &&
        std::is_constructible_v<std::decay_t<P>, P>);

// (5)
template <proxiable<F> P, class... Args>
explicit proxy(std::in_place_type_t<P>, Args&&... args)
    noexcept(std::is_nothrow_constructible_v<P, Args...>)
    requires(std::is_constructible_v<P, Args...>);

// (6)
template <proxiable<F> P, class U, class... Args>
explicit proxy(std::in_place_type_t<P>, std::initializer_list<U> il,
        Args&&... args)
    noexcept(std::is_nothrow_constructible_v<
        P, std::initializer_list<U>&, Args...>)
    requires(std::is_constructible_v<P, std::initializer_list<U>&, Args...>);
```

Creates a new `proxy`.

- `(1)` Default constructor and the constructor taking `nullptr` construct a `proxy` that does not contain a value.
- `(2)` Copy constructor constructs a `proxy` whose contained value is that of `rhs` if `rhs` contains a value, or otherwise, constructs a `proxy` that does not contain a value. As per the `requires` clause, the copy constructor is trivial when `F::constraints.copyability == constraint_level::trivial`.
- `(3)` Move constructor constructs a `proxy` whose contained value is that of `rhs` if `rhs` contains a value, or otherwise, constructs a `proxy` that does not contain a value. `rhs` is in a valid but unspecified state after move construction. As per the `requires` clause, the move constructor does not participate in overload resolution when `F::constraints.copyability == constraint_level::trivial`, so that a move construction falls back to the trivial copy constructor.
- `(4)` Let `VP` be `std::decay_t<P>`. Constructor taking a value of pointer constructs a `proxy` whose contained value is of type `VP` and direct-non-list-initialized with `std::forward<P>(ptr)`.
- `(5)` Constructs a `proxy` whose contained value is of type `P` and direct-non-list-initialized with `std::forward<Args>(args)...`.
- `(6)` Constructs a `proxy` whose contained value is of type `P` and direct-non-list-initialized with `il, std::forward<Args>(args)...`.

## Comparing with Other Standard Polymorphic Wrappers

The constructors of `proxy<F>` are similar to but have certain differences from other polymorphic wrappers in the standard, specifically, [`std::any`](https://en.cppreference.com/w/cpp/utility/any/any), and [`std::move_only_function`](https://en.cppreference.com/w/cpp/utility/functional/move_only_function/move_only_function).

[`std::function`](https://en.cppreference.com/w/cpp/utility/functional/function/function) was introduced in C++11. Comparing its constructors with `proxy`:

- It forces the target type to be copy constructible, even if its copy constructor is not used in a certain context (which motivated the introduction of `std::move_only_function` in C++23).
- It only supports `Callable` types, while `proxy` supports any pointer type that satisfies [`proxiable<P, F>`](../proxiable.md).
- It does not have overloads that take `std::in_place_type_t` to construct a value in-place.
- It does not have a conditional default copy constructor, which is efficient for trivial types.
- It used to have several overloads that took an additional allocator, but these were [removed in C++17](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0302r1.html) because "the semantics are unclear, and there are technical issues with storing an allocator in a type-erased context and then recovering that allocator later for any allocations needed during copy assignment". Although the constructors of `proxy` do not explicitly take custom allocator types, we believe this is a useful scenario and provided full support via [`allocate_proxy`](../allocate_proxy.md).
- It forces `noexcept` specifiers, which is mitigated in `std::move_only_function`.

[`std::any`](https://en.cppreference.com/w/cpp/utility/any/any) was introduced in C++17. Comparing its constructors with `proxy`:

- Similar with `std::function`, It forces the target type to be copy constructible, even if its copy constructor is not used in a certain context.
- Similar with `std::function`, it does not have a conditional default copy constructor, which is efficient for trivial types.
- It does not support allocators.
- Similar with `std::function`, it forces `noexcept` specifiers.
- In the overloads that take `std::in_place_type_t<T>` to construct a value in-place, the value type is obtained via `std::decay_t<T>` rather than bare `T`, which complicates the semantics and we do not believe is useful.

[`std::move_only_function`](https://en.cppreference.com/w/cpp/utility/functional/move_only_function/move_only_function) was introduced in C++23. Comparing its constructors with `proxy`:

- As its name suggests, it is not copyable at all. `proxy` is conditionally copyable depending on the implementation of the given facade type `F`.
- Similar with `std::function`, it only supports `Callable` types, while `proxy` support any pointer type that satisfies [`proxiable<P, F>`](../proxiable.md).
- Similar with `std::any`, it does not support allocators.
- Similar with `std::any`, in the overloads that take `std::in_place_type_t<T>` to construct a value in-place, the value type is obtained via `std::decay_t<T>` rather than bare `T`, which complicates the semantics and we do not believe is useful.

## Example

```cpp
#include <deque>
#include <iostream>
#include <vector>

#include "proxy.h"

PRO_DEF_MEM_DISPATCH(MemSize, size);
PRO_DEF_MEM_DISPATCH(MemClear, clear);

struct BasicContainer : pro::facade_builder
    ::add_convention<MemSize, std::size_t() const& noexcept>
    ::add_convention<MemClear, void() noexcept>
    ::support_copy<pro::constraint_level::nontrivial>
    ::build {};

int main() {
  std::vector<int> v{1, 2, 3};

  pro::proxy<BasicContainer> p0;
  std::cout << std::boolalpha << p0.has_value() << "\n";  // Prints "false"

  // Construct a proxy with a raw pointer
  pro::proxy<BasicContainer> p1 = &v;
  std::cout << p1.has_value() << ", " << p1->size() << "\n";  // Prints "true,3"

  // Construct a proxy with a smart pointer
  pro::proxy<BasicContainer> p2 = std::make_shared<std::deque<double>>(10);
  std::cout << p2.has_value() << ", " << p2->size() << "\n";  // Prints "true,10"

  // Copy construction
  pro::proxy<BasicContainer> p3 = p2;
  std::cout << p3.has_value() << ", " << p3->size() << "\n";  // Prints "true,10"

  // Move construction
  pro::proxy<BasicContainer> p4 = std::move(p3);
  std::cout << p4.has_value() << ", " << p4->size() << "\n";  // Prints "true,10"

  // p3 no longer contains a value
  std::cout << p3.has_value() << "\n";  // Prints "false"

  // p2 and p4 shares the same object of std::deque<double>
  p2->clear();
  std::cout << p4.has_value() << ", " << p4->size() << "\n";  // Prints "true,0"
}
```

## See Also

- [concept `proxiable`](../proxiable.md)
