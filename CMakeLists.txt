cmake_minimum_required (VERSION 3.5)

project(msft_proxy)
add_library(msft_proxy INTERFACE)
target_compile_features(msft_proxy INTERFACE cxx_std_20)
target_include_directories(msft_proxy INTERFACE .)

include(CTest)
if (BUILD_TESTING)
  include(FetchContent)
  # gtest version release-1.11.0
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/e2239ee6043f73722e7aa812a459f54a28552929.zip
  )
  
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE) # For Windows: Prevent overriding the parent project's compiler/linker settings
  set(BUILD_GMOCK OFF CACHE BOOL "" FORCE) # Disable GMock
  FetchContent_MakeAvailable(googletest)
  
  add_subdirectory(tests)
endif()
