


add_executable(msft_proxy_tests_cpp20_modules
  main.cpp
)

target_sources(msft_proxy_tests_cpp20_modules
  PRIVATE
    FILE_SET CXX_MODULES FILES
    foo.cpp
    impl.cpp
)

target_compile_features(
  msft_proxy_tests_cpp20_modules
  PRIVATE
  cxx_std_20
)

target_link_libraries(msft_proxy_tests_cpp20_modules PRIVATE msft_proxy msft_proxy::module)
