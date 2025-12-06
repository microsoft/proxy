project(msft_proxy_tests)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.tar.gz
  URL_HASH SHA256=65fab701d9829d38cb77c14acdc431d2108bfdbf8979e40eb8ae567edf10b27c
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE) # For Windows: Prevent overriding the parent project's compiler/linker settings
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE) # Disable GMock
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

add_executable(msft_proxy_tests
  proxy_creation_tests.cpp
  proxy_dispatch_tests.cpp
  proxy_fmt_format_tests.cpp
  proxy_format_tests.cpp
  proxy_integration_tests.cpp
  proxy_invocation_tests.cpp
  proxy_lifetime_tests.cpp
  proxy_reflection_tests.cpp
  proxy_regression_tests.cpp
  proxy_rtti_tests.cpp
  proxy_traits_tests.cpp
  proxy_view_tests.cpp
)
target_include_directories(msft_proxy_tests PRIVATE .)
target_link_libraries(msft_proxy_tests PRIVATE msft_proxy4::proxy gtest_main fmt::fmt)

if(MSVC)
  target_compile_options(msft_proxy_tests PRIVATE /W4 /WX)
else()
  target_compile_options(msft_proxy_tests PRIVATE -Wall -Wextra -Wpedantic -Werror $<$<CXX_COMPILER_ID:Clang>:-Wno-c++2b-extensions>)
endif()

gtest_discover_tests(msft_proxy_tests)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_executable(msft_proxy_freestanding_tests freestanding/proxy_freestanding_tests.cpp)
  target_compile_options(msft_proxy_freestanding_tests PRIVATE -ffreestanding -fno-exceptions -fno-rtti -Wall -Wextra -Wpedantic -Werror $<$<CXX_COMPILER_ID:Clang>:-Wno-c++2b-extensions>)
  target_link_options(msft_proxy_freestanding_tests PRIVATE -nodefaultlibs -lc)
  target_link_libraries(msft_proxy_freestanding_tests PRIVATE msft_proxy4::proxy)
  add_test(NAME ProxyFreestandingTests COMMAND msft_proxy_freestanding_tests)
endif()

add_subdirectory(modules)
