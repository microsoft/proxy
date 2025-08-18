project(msft_proxy_tests)

include(FetchContent)
# The policy uses the download time for timestamp, instead of the timestamp in the archive. This
# allows for proper rebuilds when a projects URL changes.
if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/releases/download/v1.15.2/googletest-1.15.2.tar.gz
  URL_HASH SHA256=7b42b4d6ed48810c5362c265a17faebe90dc2373c885e5216439d37927f02926
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE) # For Windows: Prevent overriding the parent project's compiler/linker settings
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE) # Disable GMock
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

FetchContent_Declare(
  fmt
  URL https://github.com/fmtlib/fmt/archive/refs/tags/11.1.4.tar.gz
  URL_HASH SHA256=ac366b7b4c2e9f0dde63a59b3feb5ee59b67974b14ee5dc9ea8ad78aa2c1ee1e
)
FetchContent_MakeAvailable(fmt)

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
