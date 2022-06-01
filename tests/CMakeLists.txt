project(msft_proxy_tests)
add_executable(msft_proxy_tests
  DummyTests.cpp
)
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
