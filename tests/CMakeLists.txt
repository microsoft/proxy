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

add_executable(msft_proxy_tests
  proxy_creation_tests.cpp
  proxy_dispatch_tests.cpp
  proxy_integration_tests.cpp
  proxy_invocation_tests.cpp
  proxy_lifetime_tests.cpp
  proxy_reflection_tests.cpp
  proxy_traits_tests.cpp
)
target_include_directories(msft_proxy_tests PRIVATE .)
target_link_libraries(msft_proxy_tests PRIVATE msft_proxy)
target_link_libraries(msft_proxy_tests PRIVATE gtest_main)

if(MSVC)
  target_compile_options(msft_proxy_tests PRIVATE /W4 /WX)
else()
  target_compile_options(msft_proxy_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

include(GoogleTest)
gtest_discover_tests(msft_proxy_tests)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_executable(msft_proxy_freestanding_tests freestanding/proxy_freestanding_tests.cpp)
  target_include_directories(msft_proxy_freestanding_tests PRIVATE .)
  target_compile_features(msft_proxy_freestanding_tests PRIVATE cxx_std_20)
  target_compile_options(msft_proxy_freestanding_tests PRIVATE -ffreestanding -fno-exceptions -fno-rtti -Wall -Wextra -Wpedantic -Werror)
  target_link_options(msft_proxy_freestanding_tests PRIVATE -nodefaultlibs -lc)
  target_link_libraries(msft_proxy_freestanding_tests PRIVATE msft_proxy)
  add_test(NAME ProxyFreestandingTests COMMAND msft_proxy_freestanding_tests)
endif()
