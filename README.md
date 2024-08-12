# Proxy: Next Generation Polymorphism in C++

[![Proxy-CI](https://github.com/microsoft/proxy/actions/workflows/pipeline-ci.yml/badge.svg)](https://github.com/microsoft/proxy/actions/workflows/pipeline-ci.yml)

Are you looking to simplify the lifetime management and maintenance of polymorphic objects in C++?

Do you want to write polymorphic code in C++ as easily as in [GC languages](https://en.wikipedia.org/wiki/Garbage_collection_(computer_science)) like Java or C#, without sacrificing performance?

Have you tried other polymorphic programming libraries in C++ but found them deficient?

If so, this library is for you.

## Motivation

For decades, object-based virtual tables have been the de facto implementation of runtime polymorphism in many compiled programming languages. This mechanism has certain drawbacks, including issues with lifetime management (due to varying object sizes and ownership) and reflection (balancing usability and memory allocation). To work around these drawbacks, some languages like Java or C# choose to sacrifice performance by introducing garbage collection (GC) to facilitate lifetime management, and JIT-compile the source code at runtime to generate full metadata. This library improves upon the theory of [object-oriented programming](https://en.wikipedia.org/wiki/Object-oriented_programming) (OOP) and implements it as a C++ library following the [zero-overhead principle](https://en.cppreference.com/w/cpp/language/Zero-overhead_principle). We are proposing it for inclusion in the C++ standard.

"Proxy" is a single-header, cross-platform C++20 template library for modern runtime polymorphism based on pointer semantics. It makes runtime abstraction easier in C++: not only does it save engineering effort in managing the lifetime of different types of objects (similar to other languages with GC, e.g., Java, C#), but it also supports flexible architecture design without requiring inheritance (similar to Go or Rust, and even better). Most importantly, it generates high-quality code with equal or higher performance than an equivalent implementation with virtual functions or existing polymorphic wrappers (including `std::function`, `std::move_only_function`, `std::any`, etc.) in C++ today. Please refer to the [Proxy's Frequently Asked Questions](docs/en-us/faq.md) for more background, and refer to the [specifications](docs/en-us/specifications.md) for more technical details.

## Quick Start

"Proxy" is a header-only C++20 library. To use the library, make sure your compiler meets the [minimum requirements](#compiler-req) and just include the header file [proxy.h](proxy.h) in your source code. Alternatively, you can install the library via [vcpkg](https://learn.microsoft.com/en-us/vcpkg/get_started/overview) or [conan](https://conan.io/), by searching for "proxy" (see [vcpkg.io](https://vcpkg.io/en/package/proxy) and [conan.io](https://conan.io/center/recipes/proxy)).

### Hello World

Let's get started with the following "Hello World" example:

```cpp
#include <iostream>
#include <string>

#include "proxy.h"

struct Streamable : pro::facade_builder
    ::add_convention<pro::operator_dispatch<"<<", true>, std::ostream&(std::ostream& out) const>
    ::build {};

int main() {
  std::string str = "Hello World";
  pro::proxy<Streamable> p1 = &str;
  std::cout << "p1 = " << *p1 << "\n";  // Prints: "p1 = Hello World"

  pro::proxy<Streamable> p2 = std::make_unique<int>(123);
  std::cout << "p2 = " << *p2 << "\n";  // Prints: "p2 = 123"

  pro::proxy<Streamable> p3 = pro::make_proxy<Streamable>(3.14);
  std::cout << "p3 = " << *p3 << "\n";  // Prints: "p3 = 3.14"
}
```

Here is a step-by-step explanation:

- `#include <iostream>`: For [`std::cout`](https://en.cppreference.com/w/cpp/io/cout).
- `#include <string>`: For [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string).
- `#include "proxy.h"`: For the library "Proxy". Most of the facilities of the library are defined in namespace `pro`. If the library is consumed via [vcpkg](https://learn.microsoft.com/en-us/vcpkg/get_started/overview) or [conan](https://conan.io/), this line should be changed into `#include <proxy/proxy.h>`.
- `struct Streamable : pro::facade_builder ... ::build {}`: Defines a facade type `Streamable`. The term "facade", formally defined as the [*ProFacade* requirements](docs/en-us/ProFacade.md), is how library "Proxy" models runtime abstraction. Specifically,
  - [`pro::facade_builder`](docs/en-us/basic_facade_builder.md): Provides capability to build a facade type at compile-time.
  - [`add_convention`](docs/en-us/basic_facade_builder/add_convention.md): Adds a calling convention to the build context.
  - [`pro::operator_dispatch`](docs/en-us/operator_dispatch.md)`<"<<", true>`: Specifies a dispatch for operator `<<` expressions where the primary operand (`proxy`) is on the right-hand side (specified by the second template parameter `true`). Note that polymorphism in library "Proxy" is defined by expressions rather than member functions, which is different from C++ virtual functions or other OOP languages.
  - `std::ostream&(std::ostream& out) const`: The signature of the calling convention, similar with [`std::move_only_function`](https://en.cppreference.com/w/cpp/utility/functional/move_only_function). `const` specifies that the primary operand is `const`.
  - [`build`](docs/en-us/basic_facade_builder/build.md): Builds the context into a facade type.
- [`pro::proxy`](docs/en-us/proxy.md)`<Streamable> p1 = &str`: Creates a `proxy` object from a raw pointer of `std::string`. `p1` behaves like a raw pointer, and does not have ownership of the underlying `std::string`. If the lifetime of `str` ends before `p1`, `p1` becomes dangling.
- `std::cout << *p1`: This is how it works. It prints "Hello World" because the calling convention is defined in the facade `Streamable`, so it works as if by calling `std::cout << str`.
- [`pro::proxy`](docs/en-us/proxy.md)`<Streamable> p2 = `[`std::make_unique`](https://en.cppreference.com/w/cpp/memory/unique_ptr/make_unique)`<int>(123)`: Creates a [`std::unique_ptr`](https://en.cppreference.com/w/cpp/memory/unique_ptr)`<int>` and converts to a `proxy`. Different from `p1`, `p2` has ownership of the underlying `int`, and will call the destructor of `std::unique_ptr` when `p2` is destroyed. Note that `p1` and `p2` are of the same type `pro::proxy<Streamable>`, which means you can have a function that returns `pro::proxy<Streamable>` without exposing any information about the implementation details to its caller.
- `std::cout << *p2`: Prints "123" with no surprise.
- [`pro::proxy`](docs/en-us/proxy.md)`<Streamable> p3 = `[`pro::make_proxy`](docs/en-us/make_proxy.md)`<Streamable>(3.14)`: Creates a `proxy` from a `double` without specifying the underlying pointer type. Specifically,
  - Similar with `p2`, `p3` also has ownership of the underlying `double` value, but can effectively avoid heap allocation.
  - Since the size of the underlying type (`double`) is known to be small (on major 32- or 64-bit platforms), [`pro::make_proxy`](docs/en-us/make_proxy.md) realizes the fact at compile-time, and falls back to [`pro::make_proxy_inplace`](docs/en-us/make_proxy_inplace.md), which guarantees no heap allocation.
  - Library "Proxy" explicitly defines when heap allocation occurs or not to avoid users falling into performance hell, which is different from [`std::function`](https://en.cppreference.com/w/cpp/utility/functional/function) and other existing polymorphic wrappers in the standard.

- `std::cout << *p3`: Prints "3.14" with no surprise.
- When `main` returns, `p2` and `p3` will destroy the underlying objects, while `p1` does nothing because it holds a raw pointer that does not have ownership of the underlying `std::string`.

### More Expressions

In addition to the operator expressions demonstrated in the previous example, the library supports almost all forms of expressions in C++ and can make them polymorphic. Specifically,

- [macro `PRO_DEF_MEM_DISPATCH`](docs/en-us/PRO_DEF_MEM_DISPATCH.md): Defines a dispatch type for member function call expressions.
- [macro `PRO_DEF_FREE_DISPATCH`](docs/en-us/PRO_DEF_FREE_DISPATCH.md): Defines a dispatch type for free function call expressions.
- [class template `pro::operator_dispatch`](docs/en-us/operator_dispatch.md): Dispatch type for operator expressions.
- [class template `pro::conversion_dispatch`](docs/en-us/conversion_dispatch.md): Dispatch type for conversion expressions.

Note that some facilities are provided as macro, because C++ templates today do not support generating a function with an arbitrary name. Here is another example that makes member function call expressions polymorphic:

```cpp
#include <iostream>
#include <sstream>

#include "proxy.h"

PRO_DEF_MEM_DISPATCH(MemDraw, Draw);
PRO_DEF_MEM_DISPATCH(MemArea, Area);

struct Drawable : pro::facade_builder
    ::add_convention<MemDraw, void(std::ostream& output)>
    ::add_convention<MemArea, double() noexcept>
    ::support_copy<pro::constraint_level::nontrivial>
    ::build {};

class Rectangle {
 public:
  Rectangle(double width, double height) : width_(width), height_(height) {}
  Rectangle(const Rectangle&) = default;

  void Draw(std::ostream& out) const {
    out << "{Rectangle: width = " << width_ << ", height = " << height_ << "}";
  }
  double Area() const noexcept { return width_ * height_; }

 private:
  double width_;
  double height_;
};

std::string PrintDrawableToString(pro::proxy<Drawable> p) {
  std::stringstream result;
  result << "entity = ";
  p->Draw(result);
  result << ", area = " << p->Area();
  return std::move(result).str();
}

int main() {
  pro::proxy<Drawable> p = pro::make_proxy<Drawable, Rectangle>(3, 5);
  std::string str = PrintDrawableToString(p);
  std::cout << str << "\n";  // Prints: "entity = {Rectangle: width = 3, height = 5}, area = 15"
}
```

Here is a step-by-step explanation:

- `#include <iostream>`: For [`std::cout`](https://en.cppreference.com/w/cpp/io/cout).
- `#include <sstream>`: For [`std::stringstream`](https://en.cppreference.com/w/cpp/io/basic_stringstream).
- `#include "proxy.h"`: For library "Proxy".
- [`PRO_DEF_MEM_DISPATCH`](docs/en-us/PRO_DEF_MEM_DISPATCH.md)`(MemDraw, Draw)`: Defines a dispatch type `MemDraw` for expressions of calling member function `Draw`.
- [`PRO_DEF_MEM_DISPATCH`](docs/en-us/PRO_DEF_MEM_DISPATCH.md)`(MemArea, Area)`: Defines a dispatch type `MemArea` for expressions of calling member function `Area`.
- `struct Drawable : pro::facade_builder ... ::build {}`: Defines a facade type `Drawable`. Specifically,
  - [`add_convention`](docs/en-us/basic_facade_builder/add_convention.md): Adds calling conventions to the build context.
  - [`support_copy`](docs/en-us/basic_facade_builder/support_copy.md)`<`[`pro::constraint_level`](docs/en-us/constraint_level.md)`::nontrivial>`: Specifies the underlying pointer type shall be copyable, which also makes the resulting `proxy` type copyable.
- `class Rectangle`: An implementation of `Drawable`.
- Function `PrintDrawableToString`: Converts a `Drawable` into a `std::string`. Note that this is a function rather than a function template, which means it can generate [ABI](https://en.wikipedia.org/wiki/Application_binary_interface) in a larger build system.
- `pro::proxy<Drawable> p = pro::make_proxy<Drawable, Rectangle>(3, 5)`: Creates a `proxy<Drawable>` object containing a `Rectangle`.
- `std::string str = PrintDrawableToString(p)`: Converts `p` into a `std::string`, implicitly creates a copy of `p`.
- `std::cout << str`: Prints the string.

### Other Useful Features

Library "Proxy" is a self-contained solution for runtime polymorphism in C++. There are many other capabilities documented in the [specifications](docs/en-us/specifications.md). In addition to the features mentioned above, here is a curated list of the most popular features based on user feedback:

- **Overloading**: [`facade_builder::add_convention`](docs/en-us/basic_facade_builder/add_convention.md) is more powerful than demonstrated above. It can take any number of overload types (formally, any type meeting the [*ProOverload* requirements](docs/en-us/ProOverload.md)) and perform standard overload resolution when invoking a `proxy`.
- **Facade composition**: [`facade_builder::add_facade`](docs/en-us/basic_facade_builder/add_facade.md) allows flexible composition of different abstractions.
- **Weak dispatch**: When an object does not implement a convention, and we do not want it to trigger a hard compile error, it is allowed to define a "weak dispatch" with [macro `PRO_DEF_WEAK_DISPATCH`](docs/en-us/PRO_DEF_WEAK_DISPATCH.md) from an existing dispatch type and a default implementation.
- **Allocator awareness**: [function template `allocate_proxy`](docs/en-us/allocate_proxy.md) is able to create a `proxy` from a value with any custom allocator. In C++11, [`std::function`](https://en.cppreference.com/w/cpp/utility/functional/function) and [`std::packaged_task`](https://en.cppreference.com/w/cpp/thread/packaged_task) had constructors that accepted custom allocators for performance tuning, but these were [removed in C++17](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0302r1.html) because "the semantics are unclear, and there are technical issues with storing an allocator in a type-erased context and then recovering that allocator later for any allocations needed during copy assignment". These issues do not apply to `allocate_proxy`.
- **Configurable constraints**: [`facade_builder`](docs/en-us/basic_facade_builder.md) provides full support for constraints configuration, including memory layout (by [`restrict_layout`](docs/en-us/basic_facade_builder/restrict_layout.md)), copyability (by [`support_copy`](docs/en-us/basic_facade_builder/support_copy.md)), relocatability (by [`support_relocation`](docs/en-us/basic_facade_builder/support_relocation.md)), and destructibility (by [`support_destruction`](docs/en-us/basic_facade_builder/support_destruction.md)).
- **Reflection**: `proxy` supports type-based compile-time reflection for runtime queries. Please refer to [`facade_builder::add_reflection`](docs/en-us/basic_facade_builder/add_reflection.md) and [function template `proxy_reflect`](docs/en-us/proxy_reflect.md) for more details.

## <a name="compiler-req">Minimum Requirements for Compilers</a>

| Family | Minimum version | Required flags |
| ------ | --------------- | -------------- |
| clang  | 15.0.0          | -std=c++20     |
| gcc    | 11.2            | -std=c++20     |
| MSVC   | 19.30           | /std:c++20     |

## Build and Run Tests with CMake

```
git clone https://github.com/microsoft/proxy.git
cd proxy
cmake -B build
cmake --build build -j
ctest --test-dir build -j
```

## Related Resources

- April, 2024: [Published ISO C++ proposal P3086R2: Proxy: A Pointer-Semantics-Based Polymorphism Library](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p3086r2.pdf)
- August, 2022: [proxy: Runtime Polymorphism Made Easier Than Ever](https://devblogs.microsoft.com/cppblog/proxy-runtime-polymorphism-made-easier-than-ever/)

## Contributing

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit https://cla.opensource.microsoft.com.

When you submit a pull request, a CLA bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., status check, comment). Simply follow the instructions
provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

## Trademarks

This project may contain trademarks or logos for projects, products, or services. Authorized use of Microsoft 
trademarks or logos is subject to and must follow 
[Microsoft's Trademark & Brand Guidelines](https://www.microsoft.com/en-us/legal/intellectualproperty/trademarks/usage/general).
Use of Microsoft trademarks or logos in modified versions of this project must not cause confusion or imply Microsoft sponsorship.
Any use of third-party trademarks or logos are subject to those third-party's policies.
