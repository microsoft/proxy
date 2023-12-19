# Proxy: Polymorphism in C++ Redefined

[![Proxy-CI](https://github.com/microsoft/proxy/actions/workflows/pipeline-ci.yml/badge.svg)](https://github.com/microsoft/proxy/actions/workflows/pipeline-ci.yml)

Do you want to facilitate lifetime management and maintenance of polymorphic objects in C++?

Do you want to be able to write polymorphic code in C++ as easily as in languages with GC (like Java or C#), while still having excellent runtime performance?

Have you tried other polymorphic programming libraries in C++ but found them deficient?

If so, this library is for you. 😉

For decades, object-based virtual table has been a de facto implementation of runtime polymorphism in many (compiled) programming languages. There are many drawbacks in this mechanism, including life management (because each object may have different size and ownership) and reflection (because it is hard to balance between usability and memory allocation). To workaround these drawbacks, some languages like Java or C# choose to sacrifice performance by introducing GC to facilitate lifetime management, and JIT-compile the source code at runtime to generate full metadata. We improved the theory and implemented as a C++ library without sacrificing performance, proposed to merge into the C++ standard.

The "proxy" is a single-header, cross-platform C++ library that Microsoft uses to make runtime polymorphism easier to implement and faster. Please find the design details at https://wg21.link/p0957.

## Quick start

The "proxy" is a header-only C++20 library. Once you set the language level of your compiler not earlier than C++20 and get the header file ([proxy.h](proxy.h)), you are all set. You can also install the library via [vcpkg](https://github.com/microsoft/vcpkg/), which is a C++ library manager invented by Microsoft, by searching for "proxy" (see [vcpkg.info](https://vcpkg.info/port/proxy)).

The majority of the library is defined in namespace `pro`. Some macros are provided (currently not included in the proposal of standardization) to simplify the definiton of `proxy` prior to C++26. Here is a demo showing how to use this library to implement runtime polymorphism in a different way from the traditional inheritance-based approach:

```cpp
// Abstraction (poly is short for polymorphism)
namespace poly {

PRO_DEF_MEMBER_DISPATCH(Draw, Draw, void(std::ostream&));
PRO_DEF_MEMBER_DISPATCH(Area, Area, double());
PRO_DEF_FACADE(Drawable, PRO_MAKE_DISPATCH_PACK(Draw, Area));

}  // namespace poly

// Implementation
class Rectangle {
 public:
  void Draw(std::ostream& out) const
      { out << "{Rectangle: width = " << width_ << ", height = " << height_ << "}"; }
  void SetWidth(double width) { width_ = width; }
  void SetHeight(double height) { height_ = height; }
  double Area() const { return width_ * height_; }

 private:
  double width_;
  double height_;
};

// Client - Consumer
std::string PrintDrawableToString(pro::proxy<poly::Drawable> p) {
  std::stringstream result;
  result << "shape = ";
  p.invoke<poly::Draw>(result);
  result << ", area = " << p.invoke<poly::Area>();
  return std::move(result).str();
}

// Client - Producer
pro::proxy<poly::Drawable> CreateRectangleAsDrawable(int width, int height) {
  Rectangle rect;
  rect.SetWidth(width);
  rect.SetHeight(height);
  return pro::make_proxy<poly::Drawable>(rect);
}
```

Here is another demo showing how to define overloads in a dispatch. Note that `.invoke<>` can be ommitted when only 1 dispatch is defined in a facade:

```cpp
// Abstraction (poly is short for polymorphism)
namespace poly {

PRO_DEF_MEMBER_DISPATCH(Log, Log,
    void(const char*), void(const char*, const std::exception&));
PRO_DEF_FACADE(Logger, Log);

}  // namespace poly

// Client - Consumer
void MyVerboseFunction(pro::proxy<poly::Logger> logger) {
  logger("hello");
  try {
    throw std::runtime_error{"runtime error!"};
  } catch (const std::exception& e) {
    logger("world", e);
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

Please find more details and discussions in the spec. The complete version of the "drawable" demo could be found in [tests/proxy_integration_tests.cpp](tests/proxy_integration_tests.cpp) (also available on [Compiler Explorer](https://godbolt.org/z/5a3jeE1M8)).

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
