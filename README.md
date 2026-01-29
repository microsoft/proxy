<div style="background:#fdecea;color:#2b2b2b;padding:12px 16px;border-radius:6px;margin-bottom:16px;border:1px solid #f5c2c7;">
  <strong>This repository is no longer actively maintained under the Microsoft organization.</strong>
  Active development continues at <a href="https://github.com/ngcpp/proxy">ngcpp/proxy</a>.
  <br><br>
  <strong>Why the change?</strong> Stewardship of the project has transitioned to a new independent organization, <strong>ngcpp</strong>, to ensure ongoing development and community-driven governance.
  <br><br>
  <strong>What does this mean?</strong>
  <ul>
    <li><strong>Read-only</strong>: This repository is archived and will not accept new issues or pull requests.</li>
    <li><strong>History preserved</strong>: All past commits, issues, and discussions remain available for reference.</li>
    <li><strong>Future contributions</strong>: Please direct all contributions, bug reports, and feature requests to <a href="https://github.com/ngcpp/proxy">ngcpp/proxy</a>.</li>
  </ul>
  Thank you to everyone who has supported Proxy here. We look forward to continuing the journey together at <strong>ngcpp</strong>.
</div>

# Proxy: Next Generation Polymorphism in C++

[![Proxy-CI](https://github.com/microsoft/proxy/actions/workflows/pipeline-ci.yml/badge.svg)](https://github.com/microsoft/proxy/actions/workflows/pipeline-ci.yml)

Are you looking to simplify the lifetime management and maintenance of polymorphic objects in C++?

Do you want to write polymorphic code in C++ as easily as in [GC languages](https://en.wikipedia.org/wiki/Garbage_collection_(computer_science)) like Java or C#, without sacrificing performance?

Have you tried other polymorphic programming libraries in C++ but found them deficient?

If so, this library is for you.

[![Pure Virtual C++ 2025 - Proxy - YouTube](https://img.youtube.com/vi/b_VNOq-lI4A/0.jpg)](https://www.youtube.com/watch?v=b_VNOq-lI4A)

## Our Mission

"Proxy" is a modern C++ library that helps you use polymorphism (a way to use different types of objects interchangeably) without needing inheritance.

"Proxy" was created by Microsoft engineers and has been used in the Windows operating system since 2022. For many years, using inheritance was the main way to achieve polymorphism in C++. However, new programming languages like [Rust](https://doc.rust-lang.org/book/ch10-02-traits.html) offer better ways to do this. We have improved our understanding of object-oriented programming and decided to use *pointers* in C++ as the foundation for "Proxy". Specifically, the "Proxy" library is designed to be:

- **Portable**: "Proxy" was implemented as a header-only library in standard C++20. It can be used on any platform while the compiler supports C++20. The majority of the library is [freestanding](https://en.cppreference.com/w/cpp/freestanding), making it feasible for embedded engineering or kernel design of an operating system.
- **Non-intrusive**: An implementation type is no longer required to inherit from an abstract binding.
- **Well-managed**: "Proxy" provides a GC-like capability that manages the lifetimes of different objects efficiently without the need for an actual garbage collector.
- **Fast**: With typical compiler optimizations, "Proxy" produces high-quality code that is as good as or better than hand-written code. In many cases, "Proxy" performs better than traditional inheritance-based approaches, especially in managing the lifetimes of objects.
- **Accessible**: Learned from user feedback, accessibility has been significantly improved with intuitive syntax, good IDE compatibility, and accurate diagnostics.
- **Flexible**: Not only member functions, the "abstraction" of "Proxy" allows *any* expression to be polymorphic, including free functions, operators, conversions, etc. Different abstractions can be freely composed on demand. Performance tuning is supported for experts to balance between extensibility and performance.

Please refer to the [Proxy's Frequently Asked Questions](https://microsoft.github.io/proxy/faq) for more background, and refer to the [specifications](https://microsoft.github.io/proxy/spec) for more technical details.

## Quick Start

"Proxy" is a header-only C++20 library. To use the library, make sure your compiler meets the [minimum requirements](#compiler-req) and just put the [proxy](https://github.com/microsoft/proxy/tree/main/include/proxy) directory in your project's include directory. Alternatively, you can install the library via:

- [vcpkg](https://learn.microsoft.com/en-us/vcpkg/get_started/overview): [proxy port on vcpkg.io](https://vcpkg.io/en/package/proxy)
- [conan](https://conan.io/): [proxy recipe on conan.io](https://conan.io/center/recipes/proxy)
- [CPM](https://github.com/cpm-cmake/CPM.cmake) / CMake [FetchContent_Declare](https://cmake.org/cmake/help/latest/module/FetchContent.html):

    ```cmake
    CPMAddPackage(
      NAME msft_proxy4
      GIT_TAG 4.0.0 # or above
      GIT_REPOSITORY https://github.com/microsoft/proxy.git
    )
    
    target_link_libraries(main PRIVATE msft_proxy4::proxy)
    ```

### Hello World

Let's get started with the following "Hello World" example ([run](https://godbolt.org/z/f7W36f3se)):

```cpp
#include <format>
#include <iostream>
#include <string>

#include <proxy/proxy.h>

struct Formattable : pro::facade_builder
    ::add_skill<pro::skills::format>
    ::build {};

int main() {
  static std::string str = "Hello World";
  pro::proxy<Formattable> p1 = &str;
  std::cout << std::format("*p1 = {}\n", *p1);  // Prints "*p1 = Hello World"

  pro::proxy<Formattable> p2 = std::make_unique<int>(123);
  std::cout << std::format("*p2 = {}\n", *p2);  // Prints "*p2 = 123"

  pro::proxy<Formattable> p3 = pro::make_proxy<Formattable>(3.14159);
  std::cout << std::format("*p3 = {:.2f}\n", *p3);  // Prints "*p3 = 3.14"
}
```

Here is a step-by-step explanation:

- `#include <format>`: For [`std::format`](https://en.cppreference.com/w/cpp/utility/format/format).
- `#include <iostream>`: For [`std::cout`](https://en.cppreference.com/w/cpp/io/cout).
- `#include <string>`: For [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string).
- `#include <proxy/proxy.h>`: For the "Proxy" library. Most of the facilities of the library are defined in namespace `pro`.
- `struct Formattable : pro::facade_builder ... ::build {}`: Defines a facade type `Formattable`. The term "facade", formally defined as the [*ProFacade* requirements](https://microsoft.github.io/proxy/spec/ProFacade), is how the "Proxy" library models runtime abstraction. Specifically,
  - [`pro::facade_builder`](https://microsoft.github.io/proxy/spec/basic_facade_builder): Provides capability to build a facade type at compile-time.
  - [`add_skill`](https://microsoft.github.io/proxy/spec/basic_facade_builder/add_skill)`<`[`pro::skills::format`](https://microsoft.github.io/proxy/spec/skills_format)`>`: Specifies the capability of formatting (via [standard formatting functions](https://en.cppreference.com/w/cpp/utility/format)).
  - [`build`](https://microsoft.github.io/proxy/docs/basic_facade_builder/build.html): Builds the context into a facade type.
- [`pro::proxy`](https://microsoft.github.io/proxy/spec/proxy)`<Formattable> p1 = &str`: Creates a `proxy` object from a raw pointer of `std::string`. `p1` behaves like a raw pointer, and does not have ownership of the underlying `std::string`. If the lifetime of `str` ends before `p1`, `p1` becomes dangling.
- `std::format("*p1 = {}\n", *p1)`: This is how it works. `*p1` is formatted as "Hello World" because the capability was defined in the facade `Formattable`, so it works as if by calling `std::format("*p1 = {}\n", str)`.
- [`pro::proxy`](https://microsoft.github.io/proxy/spec/proxy)`<Formattable> p2 = `[`std::make_unique`](https://en.cppreference.com/w/cpp/memory/unique_ptr/make_unique)`<int>(123)`: Creates a [`std::unique_ptr`](https://en.cppreference.com/w/cpp/memory/unique_ptr)`<int>` and converts to a `proxy`. Different from `p1`, `p2` has ownership of the underlying `int` because it is instantiated from a value of `std::unique_ptr`, and will call the destructor of `std::unique_ptr` when `p2` is destroyed, while `p1` does not have ownership of the underlying `int` because it is instantiated from a raw pointer. `p1` and `p2` are of the same type `pro::proxy<Formattable>`, which means you can have a function that returns `pro::proxy<Formattable>` without exposing any information about the implementation details to its caller.
- `std::format("*p2 = {}\n", *p2)`: Formats `*p2` as "123" with no surprises.
- [`pro::proxy`](https://microsoft.github.io/proxy/spec/proxy)`<Formattable> p3 = `[`pro::make_proxy`](https://microsoft.github.io/proxy/spec/make_proxy)`<Formattable>(3.14159)`: Creates a `proxy` from a `double` without specifying the underlying pointer type. Specifically,
  - Similar with `p2`, `p3` also has ownership of the underlying `double` value, but can effectively avoid heap allocation.
  - Since the size of the underlying type (`double`) is known to be small (on major 32- or 64-bit platforms), [`pro::make_proxy`](https://microsoft.github.io/proxy/spec/make_proxy) realizes the fact at compile-time, and falls back to [`pro::make_proxy_inplace`](https://microsoft.github.io/proxy/spec/make_proxy_inplace), which guarantees no heap allocation.
  - The "Proxy" library explicitly defines when heap allocation occurs or not to avoid users falling into performance hell, which is different from [`std::function`](https://en.cppreference.com/w/cpp/utility/functional/function) and other existing polymorphic wrappers in the standard.
- `std::format("*p3 = {:.2f}\n", *p3)`: Formats `*p3` as "3.14" as per the [standard format specification](https://en.cppreference.com/w/cpp/utility/format/spec) with no surprises.
- When `main` returns, `p2` and `p3` will destroy the underlying objects, while `p1` does nothing because it holds a raw pointer that does not have ownership of the underlying `std::string`.

Note: If you prefer the library to be consumed as a (C++20) module, refer to [C++20 Modules support](https://microsoft.github.io/proxy/modules_support).

### Hello World (Stream Version)

In the previous "Hello World" example, we demonstrated how `proxy` could manage different types of objects and be formatted with `std::format`. While `std::format` is not the only option to print objects in C++, can we simply make `proxy` work with `std::cout`? The answer is "yes". The previous example is equivalent to the following implementation ([run](https://godbolt.org/z/447aMbrbj)):

```cpp
#include <iomanip>
#include <iostream>
#include <string>

#include <proxy/proxy.h>

struct Streamable : pro::facade_builder
    ::add_convention<pro::operator_dispatch<"<<", true>, std::ostream&(std::ostream& out) const>
    ::build {};

int main() {
  static std::string str = "Hello World";
  pro::proxy<Streamable> p1 = &str;
  std::cout << "*p1 = " << *p1 << "\n";  // Prints "p1 = Hello World"

  pro::proxy<Streamable> p2 = std::make_unique<int>(123);
  std::cout << "*p2 = " << *p2 << "\n";  // Prints "p2 = 123"

  pro::proxy<Streamable> p3 = pro::make_proxy<Streamable>(3.14159);
  std::cout << "*p3 = " << std::fixed << std::setprecision(2) << *p3 << "\n";  // Prints "p3 = 3.14"
}
```

Here is a step-by-step explanation:

- `#include <iomanip>`: For [`std::setprecision`](https://en.cppreference.com/w/cpp/io/manip/setprecision).
- `#include <iostream>`: For [`std::cout`](https://en.cppreference.com/w/cpp/io/cout).
- `#include <string>`: For [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string).
- `#include <proxy/proxy.h>`: For the "Proxy" library.
- `struct Streamable : pro::facade_builder ... ::build {}`: Defines a facade type `Streamable`. Specifically,
  - [`pro::facade_builder`](https://microsoft.github.io/proxy/spec/basic_facade_builder): Gets prepared to build another facade.
  - [`add_convention`](https://microsoft.github.io/proxy/spec/basic_facade_builder/add_convention): Adds a generalized "calling convention", defined by a "dispatch" and several "overloads", to the build context.
  - [`pro::operator_dispatch`](https://microsoft.github.io/proxy/spec/operator_dispatch)`<"<<", true>`: Specifies a dispatch for operator `<<` expressions where the primary operand (`proxy`) is on the right-hand side (specified by the second template parameter `true`). Note that polymorphism in the "Proxy" library is defined by expressions rather than member functions, which is different from C++ virtual functions or other OOP languages.
  - `std::ostream&(std::ostream& out) const`: The signature of the calling convention, similar with [`std::move_only_function`](https://en.cppreference.com/w/cpp/utility/functional/move_only_function). `const` specifies that the primary operand is `const`.
  - [`build`](https://microsoft.github.io/proxy/spec/basic_facade_builder/build): Builds the context into a facade type.

- [`pro::proxy`](https://microsoft.github.io/proxy/spec/proxy)`<Streamable> p1 = &str`: Creates a `proxy` object from a raw pointer of `std::string`.
- `std::cout << *p1`: It prints "Hello World" because the calling convention is defined in the facade `Streamable`, so it works as if by calling `std::cout << str`.
- [`pro::proxy`](https://microsoft.github.io/proxy/spec/proxy)`<Streamable> p2 = `[`std::make_unique`](https://en.cppreference.com/w/cpp/memory/unique_ptr/make_unique)`<int>(123)`: Creates a [`std::unique_ptr`](https://en.cppreference.com/w/cpp/memory/unique_ptr)`<int>` and converts to a `proxy`.
- `std::cout << *p2`: Prints "123" with no surprises.
- [`pro::proxy`](https://microsoft.github.io/proxy/spec/proxy)`<Streamable> p3 = `[`pro::make_proxy`](https://microsoft.github.io/proxy/spec/make_proxy)`<Streamable>(3.14)`: Creates a `proxy` from a `double`.
- `std::cout << std::fixed << std::setprecision(2) << *p3;`: Prints "3.14" with no surprises.

### More Expressions

In addition to the operator expressions demonstrated in the previous examples, the library supports almost all forms of expressions in C++ and can make them polymorphic. Specifically,

- [macro `PRO_DEF_MEM_DISPATCH`](https://microsoft.github.io/proxy/spec/PRO_DEF_MEM_DISPATCH): Defines a dispatch type for member function call expressions, providing accessibility as member functions.
- [macro `PRO_DEF_FREE_DISPATCH`](https://microsoft.github.io/proxy/spec/PRO_DEF_FREE_DISPATCH): Defines a dispatch type for free function call expressions, providing accessibility as free functions.
- [macro `PRO_DEF_FREE_AS_MEM_DISPATCH`](https://microsoft.github.io/proxy/spec/PRO_DEF_FREE_AS_MEM_DISPATCH): Defines a dispatch type for free function call expressions, providing accessibility as member functions.
- [class template `pro::operator_dispatch`](https://microsoft.github.io/proxy/spec/operator_dispatch): Dispatch type for operator expressions.
- [class `explicit_conversion_dispatch` (aka. `conversion_dispatch`)](https://microsoft.github.io/proxy/spec/explicit_conversion_dispatch) and [class `implicit_conversion_dispatch`](https://microsoft.github.io/proxy/spec/implicit_conversion_dispatch): Dispatch type for conversion expressions.

Note that some facilities are provided as macro, because C++ templates today do not support generating a function with an arbitrary name. Here is another example that makes member function call expressions polymorphic ([run](https://godbolt.org/z/E95nY7PYq)):

```cpp
#include <iostream>
#include <sstream>

#include <proxy/proxy.h>

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
  std::cout << str << "\n";  // Prints "entity = {Rectangle: width = 3, height = 5}, area = 15"
}
```

Here is a step-by-step explanation:

- `#include <iostream>`: For [`std::cout`](https://en.cppreference.com/w/cpp/io/cout).
- `#include <sstream>`: For [`std::stringstream`](https://en.cppreference.com/w/cpp/io/basic_stringstream).
- `#include <proxy/proxy.h>`: For the "Proxy" library.
- [`PRO_DEF_MEM_DISPATCH`](https://microsoft.github.io/proxy/spec/PRO_DEF_MEM_DISPATCH)`(MemDraw, Draw)`: Defines a dispatch type `MemDraw` for expressions of calling member function `Draw`.
- [`PRO_DEF_MEM_DISPATCH`](https://microsoft.github.io/proxy/spec/PRO_DEF_MEM_DISPATCH)`(MemArea, Area)`: Defines a dispatch type `MemArea` for expressions of calling member function `Area`.
- `struct Drawable : pro::facade_builder ... ::build {}`: Defines a facade type `Drawable`. Specifically,
  - [`add_convention`](https://microsoft.github.io/proxy/spec/basic_facade_builder/add_convention): Adds calling conventions to the build context.
  - [`support_copy`](https://microsoft.github.io/proxy/spec/basic_facade_builder/support_copy)`<`[`pro::constraint_level`](https://microsoft.github.io/proxy/spec/constraint_level)`::nontrivial>`: Specifies the underlying pointer type shall be copyable, which also makes the resulting `proxy` type copyable.
- `class Rectangle`: An implementation of `Drawable`.
- Function `PrintDrawableToString`: Converts a `Drawable` into a `std::string`. Note that this is a function rather than a function template, which means it can generate [ABI](https://en.wikipedia.org/wiki/Application_binary_interface) in a larger build system.
- `pro::proxy<Drawable> p = pro::make_proxy<Drawable, Rectangle>(3, 5)`: Creates a `proxy<Drawable>` object containing a `Rectangle`.
- `std::string str = PrintDrawableToString(p)`: Converts `p` into a `std::string`, implicitly creates a copy of `p`.
- `std::cout << str`: Prints the string.

### Other Useful Features

The "Proxy" library is a self-contained solution for runtime polymorphism in C++. There are many other capabilities documented in the [specifications](https://microsoft.github.io/proxy/spec). In addition to the features mentioned above, here is a curated list of the most popular features based on user feedback:

- **Overloading**: [`facade_builder::add_convention`](https://microsoft.github.io/proxy/spec/basic_facade_builder/add_convention) is more powerful than demonstrated above. It can take any number of overload types (formally, any type meeting the [*ProOverload* requirements](https://microsoft.github.io/proxy/spec/ProOverload)) and perform standard overload resolution when invoking a `proxy`.
- **Facade composition**: [`facade_builder::add_facade`](https://microsoft.github.io/proxy/spec/basic_facade_builder/add_facade) allows flexible composition of different abstractions.
- **Concepts**: To facilitate template programming with "Proxy", 3 concepts are exported from a facade type. Namely, [`proxiable`](https://microsoft.github.io/proxy/spec/proxiable), [`proxiable_target`](https://microsoft.github.io/proxy/spec/proxiable_target) and [`inplace_proxiable_target`](https://microsoft.github.io/proxy/spec/inplace_proxiable_target).
- **Allocator awareness**: [function template `allocate_proxy`](https://microsoft.github.io/proxy/spec/allocate_proxy) is able to create a `proxy` from a value with any custom allocator. In C++11, [`std::function`](https://en.cppreference.com/w/cpp/utility/functional/function) and [`std::packaged_task`](https://en.cppreference.com/w/cpp/thread/packaged_task) had constructors that accepted custom allocators for performance tuning, but these were [removed in C++17](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0302r1.html) because "the semantics are unclear, and there are technical issues with storing an allocator in a type-erased context and then recovering that allocator later for any allocations needed during copy assignment". These issues do not apply to `allocate_proxy`.
- **Configurable constraints**: [`facade_builder`](https://microsoft.github.io/proxy/spec/basic_facade_builder) provides full support for constraints configuration, including memory layout (by [`restrict_layout`](https://microsoft.github.io/proxy/spec/basic_facade_builder/restrict_layout)), copyability (by [`support_copy`](https://microsoft.github.io/proxy/spec/basic_facade_builder/support_copy)), relocatability (by [`support_relocation`](https://microsoft.github.io/proxy/spec/basic_facade_builder/support_relocation)), and destructibility (by [`support_destruction`](https://microsoft.github.io/proxy/spec/basic_facade_builder/support_destruction)).
- **Reflection**: `proxy` supports type-based compile-time reflection for runtime queries. Please refer to [`facade_builder::add_reflection`](https://microsoft.github.io/proxy/spec/basic_facade_builder/add_reflection) and [function template `proxy_reflect`](https://microsoft.github.io/proxy/spec/proxy_reflect) for more details.
- **Non-owning proxy**: Although `proxy` can manage the lifetime of an object effectively, similar to a smart pointer, we sometimes want to dereference it before passing to a non-owning context. This has been implemented as an extension since 3.2.0. Please refer to [function template `make_proxy_view`](https://microsoft.github.io/proxy/spec/make_proxy_view), [alias template `proxy_view`, class template `observer_facade`](https://microsoft.github.io/proxy/spec/proxy_view) and [`skills::as_view`](https://microsoft.github.io/proxy/spec/skills_as_view) for more details.
- **RTTI**: [RTTI (run-time type information)](https://en.wikipedia.org/wiki/Run-time_type_information) provides "weak" reflection capability in C++ since the last century. Although it is not as powerful as reflection in some other languages (like `Object.GetType()` in C# or `Object.getClass()` in Java), it offers the basic infrastructure for type-safe casting at runtime. Since 4.0.0, "RTTI for `proxy`" has been implemented as an extension and allows users to opt-in for each facade definition. Please refer to [`skills::rtti`](https://microsoft.github.io/proxy/spec/skills_rtti) for more details.
- **Shared and weak ownership**: Although `proxy` can be created from a [`std::shared_ptr`](https://en.cppreference.com/w/cpp/memory/shared_ptr), extensions are available to create `proxy` objects with shared and weak ownership in a more efficient way since 3.3.0. Please refer to [function template `make_proxy_shared`](https://microsoft.github.io/proxy/spec/make_proxy_shared), [`allocate_proxy_shared`](https://microsoft.github.io/proxy/spec/allocate_proxy_shared), [alias template `weak_proxy`, class template `weak_facade`](https://microsoft.github.io/proxy/spec/weak_proxy) and [`skills::as_weak`](https://microsoft.github.io/proxy/spec/skills_as_weak) for more details.
- **Weak dispatch**: When an object does not implement a convention, and we do not want it to trigger a hard compile error, it is allowed to specify a [`weak_dispatch`](https://microsoft.github.io/proxy/spec/weak_dispatch) that throws when invoked.

## <a name="compiler-req">Minimum Requirements for Compilers</a>

| Family       | Minimum version | Required flags |
| ------------ | --------------- | -------------- |
| GCC          | 13.1            | -std=c++20     |
| Clang        | 16.0.0          | -std=c++20     |
| MSVC         | 19.31           | /std:c++20     |
| NVIDIA HPC   | 24.1            | -std=c++20     |
| Intel oneAPI | 2024.0          | -std=c++20     |

## Build and Run Tests with CMake

```
git clone https://github.com/microsoft/proxy.git
cd proxy
cmake -B build
cmake --build build -j
ctest --test-dir build -j
```

## Related Resources

- August, 2025: [Announcing Proxy 4: The Next Leap in C++ Polymorphism](https://devblogs.microsoft.com/cppblog/announcing-proxy-4-the-next-leap-in-c-polymorphism/)
- May, 2025: [Published ISO C++ proposal P3086R34 Proxy: A Pointer-Semantics-Based Polymorphism Library](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3086r4.pdf)
- January, 2025: [Published ISO C++ proposal P3584R0: Enrich Facade Creation Facilities for the Pointer-Semantics-Based Polymorphism Library - Proxy](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3584r0.pdf)
- November, 2024: [Analyzing the Performance of the “Proxy” Library](https://devblogs.microsoft.com/cppblog/analyzing-the-performance-of-the-proxy-library/)
- September, 2024: [Published ISO C++ proposal P3401R0: Enrich Creation Functions for the Pointer-Semantics-Based Polymorphism Library - Proxy](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p3401r0.pdf)
- September, 2024: [Announcing the Proxy 3 Library for Dynamic Polymorphism](https://devblogs.microsoft.com/cppblog/announcing-the-proxy-3-library-for-dynamic-polymorphism/)
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
