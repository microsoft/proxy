cmake_minimum_required (VERSION 3.5)

project(msft-proxy)
add_library(msft-proxy INTERFACE)
target_compile_features(msft-proxy INTERFACE cxx_std_20)
target_include_directories(msft-proxy INTERFACE .)

include(FetchContent)
# gtest version release-1.11.0
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/e2239ee6043f73722e7aa812a459f54a28552929.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_subdirectory(tests)
