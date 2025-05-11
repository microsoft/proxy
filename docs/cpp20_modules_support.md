# C++20 Modules support

proxy ships with `.cppm` files starting with version **3.4.0**. Compared to traditional headers, modules offer faster compilation speed and isolation against preprocessor macro definitions.

As of 2025-05-11, CMake lacks support for forward compatibility when consuming C++ modules, which causes consumers with newer C++ standard to be unable to use modules with older standard. Until this is implemented by CMake, a CMake target containing the module can be manually using the following CMake script:

```cmake
find_package(proxy REQUIRED)

if(NOT DEFINED proxy_INCLUDE_DIR)
    message(FATAL_ERROR "proxy_INCLUDE_DIR must be defined to use this script.")
endif()

message(STATUS "Declaring `msft_proxy::module` target for include path ${proxy_INCLUDE_DIR}")
add_library(msft_proxy_module)
set_target_properties(
    msft_proxy_module 
    PROPERTIES
        SYSTEM TRUE
        EXCLUDE_FROM_ALL TRUE
)

add_library(msft_proxy::module ALIAS msft_proxy_module)
target_sources(msft_proxy_module PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS ${proxy_INCLUDE_DIR}
    FILES
        ${proxy_INCLUDE_DIR}/proxy/proxy.cppm
        ${proxy_INCLUDE_DIR}/proxy/proxy_interface.cppm
)
target_compile_features(msft_proxy_module PUBLIC cxx_std_26)
target_link_libraries(msft_proxy_module PUBLIC msft_proxy)
```

It can then be consumed like this:

```cmake
target_link_libraries(main PRIVATE msft_proxy::module)
```

## Example

```cpp
#include <vector>
#include <iostream>

#include <proxy/proxy_macros.h> // [1]

import proxy; // [2]
import proxy_interface; // [3]

extern "C++" { // [4]
PRO_DEF_MEM_DISPATCH(MemAt, at);
}

struct Dictionary : pro::facade_builder
    ::add_convention<MemAt, std::string(int index) const>
    ::build {};

int main() {
  std::vector<const char*> v{"hello", "world"};
  pro::proxy<Dictionary> p = &v;
  std::cout << p->at(1) << "\n";  // Prints "world"
}
```

[1] This makes all `PRO_DEF_` macros available. This header file contains only some macros and are therefore very fast to compile.
[2] `import proxy;` makes all public interfaces from `pro` namespace available in the current translation unit.
[3] `import proxy_interface;` makes some implementation details that are used by the `PRO_DEF` macros available in the current translation unit. This is only required when declaring a facade.
[4] As of 2025-05-11, clangd requires the accessor struct to be either `export`-ed, or be declared within an `extern "C++"` block, in order to have auto completion working.

