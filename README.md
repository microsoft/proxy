# Proxy: Next Generation Polymorphism in C++

[![Proxy-CI](https://github.com/microsoft/proxy/actions/workflows/pipeline-ci.yml/badge.svg)](https://github.com/microsoft/proxy/actions/workflows/pipeline-ci.yml)

Do you want to facilitate lifetime management and maintenance of polymorphic objects in C++?

Do you want to be able to write polymorphic code in C++ as easily as in languages with GC (like Java or C#), while still having excellent runtime performance?

Have you tried other polymorphic programming libraries in C++ but found them deficient?

If so, this library is for you. 😉

For decades, object-based virtual table has been a de facto implementation of runtime polymorphism in many (compiled) programming languages. There are many drawbacks in this mechanism, including life management (because each object may have different size and ownership) and reflection (because it is hard to balance between usability and memory allocation). To workaround these drawbacks, some languages like Java or C# choose to sacrifice performance by introducing GC to facilitate lifetime management, and JIT-compile the source code at runtime to generate full metadata. We improved the theory and implemented as a C++ library without sacrificing performance, proposed to merge into the C++ standard.

The "proxy" is a single-header, cross-platform C++ library that Microsoft uses to make runtime polymorphism easier to implement and faster. Please find the design details at https://wg21.link/p3086.

## Quick start

The "proxy" is a header-only C++20 library. Once you set the language level of your compiler not earlier than C++20 and get the header file ([proxy.h](proxy.h)), you are all set. You can also install the library via [vcpkg](https://github.com/microsoft/vcpkg/), which is a C++ library manager invented by Microsoft, by searching for "proxy" (see [vcpkg.info](https://vcpkg.info/port/proxy)).

The majority of the library is defined in namespace `pro`. Some macros are provided (currently not included in the proposal for standardization) to simplify the definition of `proxy` prior to C++26. Here is a demo showing how to use this library to implement runtime polymorphism in a different way from the traditional inheritance-based approach:

```cpp
// Specifications of abstraction
namespace spec {

PRO_DEF_MEMBER_DISPATCH(Draw, void(std::ostream& out));
PRO_DEF_MEMBER_DISPATCH(Area, double() noexcept);
PRO_DEF_FACADE(Drawable, PRO_MAKE_DISPATCH_PACK(Draw, Area));

}  // namespace spec

// Implementation
class Rectangle {
 public:
  void Draw(std::ostream& out) const
      { out << "{Rectangle: width = " << width_ << ", height = " << height_ << "}"; }
  void SetWidth(double width) { width_ = width; }
  void SetHeight(double height) { height_ = height; }
  double Area() const noexcept { return width_ * height_; }

 private:
  double width_;
  double height_;
};

// Client - Consumer
std::string PrintDrawableToString(pro::proxy<spec::Drawable> p) {
  std::stringstream result;
  result << "shape = ";
  p.Draw(result);  // Polymorphic call
  result << ", area = " << p.Area();  // Polymorphic call
  return std::move(result).str();
}

// Client - Producer
pro::proxy<spec::Drawable> CreateRectangleAsDrawable(int width, int height) {
  Rectangle rect;
  rect.SetWidth(width);
  rect.SetHeight(height);
  return pro::make_proxy<spec::Drawable>(rect);
}
```

Here is another demo showing how to define overloads in a dispatch: just to add any number of signatures in the definition of a dispatch.

```cpp
// Specifications of abstraction
namespace spec {

PRO_DEF_MEMBER_DISPATCH(Log, void(const char*), void(const char*, const std::exception&));
PRO_DEF_FACADE(Logger, Log);

}  // namespace spec

// Client - Consumer
void MyVerboseFunction(pro::proxy<spec::Logger> logger) {
  logger.Log("hello");
  try {
    throw std::runtime_error{"runtime error!"};
  } catch (const std::exception& e) {
    logger.Log("world", e);
  }
}

// Implementation
struct MyLogger {
  void Log(const char* s) {
    printf("[INFO] %s\n", s);
  }
  void Log(const char* s, const std::exception& e) {
    printf("[ERROR] %s (exception info: %s)\n", s, e.what());
  }
};

// Client - Producer
int main() {
  MyLogger logger;
  MyVerboseFunction(&logger);
  return 0;
}
```

By design, the body of a dispatch could be any code. While member function is one useful pattern supported by macro `PRO_DEF_MEMBER_DISPATCH`, free function is also supported with another macro `PRO_DEF_FREE_DISPATCH`. The following example uses `PRO_DEF_FREE_DISPATCH` and `std::invoke` to implement similar function wrapper as `std::function` and `std::move_only_function` and supports multiple overloads.  Note that `.Call` can be omitted when only 1 dispatch is defined in a facade:

```cpp
// Specifications of abstraction
namespace spec {

template <class... Overloads>
PRO_DEF_FREE_DISPATCH(Call, std::invoke, Overloads...);
template <class... Overloads>
PRO_DEF_FACADE(MovableCallable, Call<Overloads...>);
template <class... Overloads>
PRO_DEF_FACADE(CopyableCallable, Call<Overloads...>, pro::copyable_ptr_constraints);

}  // namespace spec

// MyFunction has similar functionality as std::function but supports multiple overloads
// MyMoveOnlyFunction has similar functionality as std::move_only_function but supports multiple overloads
template <class... Overloads>
using MyFunction = pro::proxy<spec::CopyableCallable<Overloads...>>;
template <class... Overloads>
using MyMoveOnlyFunction = pro::proxy<spec::MovableCallable<Overloads...>>;

int main() {
  auto f = [](auto&&... v) {
    printf("f() called. Args: ");
    ((std::cout << v << ":" << typeid(decltype(v)).name() << ", "), ...);
    puts("");
  };
  MyFunction<void(int)> p0{&f};
  p0(123);  // Prints "f() called. Args: 123:i," (assuming GCC)
  MyMoveOnlyFunction<void(), void(int), void(double)> p1{&f};
  p1();  // Prints "f() called. Args:"
  p1(456);  // Prints "f() called. Args: 456:i,"
  p1(1.2);  // Prints "f() called. Args: 1.2:d,"
  return 0;
}
```

Please find more details and discussions in the spec. The complete version of the "drawable" demo could be found in [tests/proxy_integration_tests.cpp](tests/proxy_integration_tests.cpp) (also available on [Compiler Explorer](https://godbolt.org/z/4cK8PPTE1)).

## Minimum requirements for compilers

| Family | Minimum version | Required flags |
| --- | --- | --- |
| clang | 15.0.0 | -std=c++20 |
| gcc | 11.2 | -std=c++20 |
| MSVC | 19.30 | /std:c++20 |

## Use `proxy` with CMake and [Vcpkg](https://github.com/microsoft/vcpkg)

See more details in [samples](./samples)

1. Set up vcpkg manifest
```
{
  "name": "<project_name>",
  "version": "0.1.0",
  "dependencies": [
    {
      "name": "proxy"
    }
  ]
}
```

2. Integrate `proxy` in CMakeLists.txt
```
find_package(proxy CONFIG REQUIRED)
target_link_libraries(<target_name> PRIVATE msft_proxy)
```

3. Run CMake with vcpkg toolchain file
```
cmake <source_dir> -B <build_dir> -DCMAKE_TOOLCHAIN_FILE=<vcpkg_dir>/scripts/buildsystems/vcpkg.cmake
```

## Build and run tests with CMake

```
git clone https://github.com/microsoft/proxy.git
cd proxy
cmake -S . -B build
cmake --build ./build -j8
cd ./build
ctest -j8
```

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
