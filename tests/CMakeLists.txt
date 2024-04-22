project(msft_proxy_tests)
add_executable(msft_proxy_tests
  proxy_creation_tests.cpp
  proxy_integration_tests.cpp
  proxy_invocation_tests.cpp
  proxy_lifetime_tests.cpp
  proxy_reflection_tests.cpp
  proxy_traits_tests.cpp
)
target_include_directories(msft_proxy_tests PRIVATE .)
target_compile_features(msft_proxy_tests PRIVATE cxx_std_20)
target_link_libraries(msft_proxy_tests PRIVATE msft_proxy)
target_link_libraries(msft_proxy_tests PRIVATE gtest_main)

if (MSVC)
  target_compile_options(msft_proxy_tests PRIVATE /W4 /WX)
else()
  target_compile_options(msft_proxy_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

include(GoogleTest)
gtest_discover_tests(msft_proxy_tests)

if(NOT MSVC)
  add_executable(msft_proxy_freestanding_tests freestanding/proxy_freestanding_tests.cpp)
  target_include_directories(msft_proxy_freestanding_tests PRIVATE .)
  target_compile_features(msft_proxy_freestanding_tests PRIVATE cxx_std_20)
  target_compile_options(msft_proxy_freestanding_tests PRIVATE -ffreestanding -fno-exceptions -fno-rtti -Wall -Wextra -Wpedantic -Werror)
  target_link_options(msft_proxy_freestanding_tests PRIVATE -nodefaultlibs -lc)
  target_link_libraries(msft_proxy_freestanding_tests PRIVATE msft_proxy)
  add_test(NAME ProxyFreestandingTests COMMAND msft_proxy_freestanding_tests)
endif()
