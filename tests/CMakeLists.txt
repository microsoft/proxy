project(msft-proxy-tests)
add_executable(msft-proxy-tests
  DummyTests.cpp
)
target_compile_features(msft-proxy-tests INTERFACE cxx_std_20)
target_link_libraries(msft-proxy-tests PRIVATE msft-proxy)
target_link_libraries(msft-proxy-tests PRIVATE gtest_main)

include(GoogleTest)
gtest_discover_tests(msft-proxy-tests)
