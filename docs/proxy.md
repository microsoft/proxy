# Class template `proxy`

```cpp
template <class F>
class proxy;
```

Class template `proxy` is a general-purpose polymorphic wrapper for C++ objects. Unlike other polymorphic wrappers in the C++ standard (e.g., [`std::function`](https://en.cppreference.com/w/cpp/utility/functional/function), [`std::move_only_function`](https://en.cppreference.com/w/cpp/utility/functional/move_only_function), [`std::any`](https://en.cppreference.com/w/cpp/utility/any), etc.), `proxy` is based on pointer semantics. It supports flexible lifetime management without runtime [garbage collection (GC)](https://en.wikipedia.org/wiki/Garbage_collection_(computer_science)) at runtime, and offers best-in-class code generation quality, extendibility and accessibility.

To instantiate `proxy<F>`, `F` shall model [concept `facade`](facade.md). As per `facade<F>`, `typename F::convention_types` shall be a [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type containing any number of distinct types `Cs`, and `typename F::reflection_types` shall be a [tuple-like](https://en.cppreference.com/w/cpp/utility/tuple/tuple-like) type containing any number of distinct types `Rs`. For each type `T` in `Cs` or `Rs`, if `T` meets the [*ProAccessible* requirements](ProAccessible.md) of `F`, `typename T::template accessor<F>` is inherited by `proxy<F>` when `T::is_direct` is `true`. Otherwise, it is inherited by the return type of [`operator*`](proxy/indirection.md) when `T::is_direct` is `false`. Implementation of accessors can call [`access_proxy`](access_proxy.md) to access the `proxy` object. It is recommended to use [`facade_builder`](basic_facade_builder.md) to define a facade type.

Any instance of `proxy<F>` at any given point in time either *contains a value* or *does not contain a value*. If a `proxy<F>` *contains a value*, the type of the value shall be a pointer type `P`  where [`proxiable<P, F>`](proxiable.md) is `true`, and the value is guaranteed to be allocated as part of the `proxy` object footprint, i.e. no dynamic memory allocation occurs. However, `P` may allocate during its construction, depending on its implementation.

## Member functions

| Name                                                       | Description                                        |
| ---------------------------------------------------------- | -------------------------------------------------- |
| [(constructor)](proxy/constructor.md)                      | constructs a `proxy` object                        |
| [(destructor)](proxy/destructor.md)                        | destroys a `proxy` object                          |
| [`operator=`](proxy/assignment.md)                         | assigns a `proxy` object                           |
| [`operator bool`<br />`has_value`](proxy/operator_bool.md) | checks if the `proxy` contains a value             |
| [`reset`](proxy/reset.md)                                  | destroys any contained value                       |
| [`swap`](proxy/swap.md)                                    | exchanges the contents                             |
| [`emplace`](proxy/emplace.md)                              | constructs the contained value in-place            |
| [`operator->`<br />`operator*`](proxy/indirection.md)      | accesses the accessors of the indirect conventions |

## Non-member functions

| Name                                              | Description                                                  |
| ------------------------------------------------- | ------------------------------------------------------------ |
| [`swap`](proxy/friend_swap.md)                    | overload the [`std::swap`](https://en.cppreference.com/w/cpp/algorithm/swap) algorithm |
| [`operator==`](proxy/friend_operator_equality.md) | compares a `proxy` with `nullptr`                            |

## Comparing with Other Standard Polymorphic Wrappers

The C++ standard includes several polymorphic wrappers, such as [`std::function`](https://en.cppreference.com/w/cpp/utility/functional/function/function), [`std::packaged_task`](https://en.cppreference.com/w/cpp/thread/packaged_task), [`std::any`](https://en.cppreference.com/w/cpp/utility/any/any), and [`std::move_only_function`](https://en.cppreference.com/w/cpp/utility/functional/move_only_function/move_only_function) (as of C++23). `proxy` offers all their useful features and more, with equal or better code generation compared to various STL implementations.

A key difference is that `proxy` is based on pointer semantics, allowing flexible lifetime management without runtime GC overhead. In C++11, `std::function` and `std::packaged_task` had constructors that accepted custom allocators for performance tuning, but these were [removed in C++17](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0302r1.html) because "the semantics are unclear, and there are technical issues with storing an allocator in a type-erased context and then recovering that allocator later for any allocations needed during copy assignment". These issues do not apply to `proxy` which fully supports custom allocators via [`allocate_proxy`](allocate_proxy.md).

Another major difference is that `proxy` is open to abstractions. Unlike `std::function`, `std::packaged_task` and `std::move_only_function`, which only abstracts `operator()`, and `std::any`, which only abstracts casting, `proxy` allows users to define any runtime abstraction requirements via [`facade`](facade.md). It is recommended to use [`facade_builder`](basic_facade_builder.md) to define a custom facade with any conventions, reflections, or constraints.

## Example

```cpp
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "proxy.h"

PRO_DEF_MEM_DISPATCH(MemAt, at);

struct Dictionary : pro::facade_builder
    ::add_convention<MemAt, std::string(int)>
    ::build {};

// This is a function, rather than a function template
void PrintDictionary(pro::proxy<Dictionary> dictionary) {
  std::cout << dictionary->at(1) << "\n";
}

int main() {
  static std::map<int, std::string> container1{{1, "hello"}};
  auto container2 = std::make_shared<std::vector<const char*>>();
  container2->push_back("hello");
  container2->push_back("world");
  PrintDictionary(&container1);  // Prints: "hello"
  PrintDictionary(container2);  // Prints: "world"
}
```

## See Also

- [concept `proxiable`](proxiable.md)
- [function template `make_proxy`](make_proxy.md)
- [macro `PRO_DEF_MEM_DISPATCH`](PRO_DEF_MEM_DISPATCH.md)
