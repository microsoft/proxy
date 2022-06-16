cmake_minimum_required (VERSION 3.5)

project(msft_proxy)
add_library(msft_proxy INTERFACE)
target_compile_features(msft_proxy INTERFACE cxx_std_20)
target_include_directories(msft_proxy INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                                                $<INSTALL_INTERFACE:include>)

include(CMakePackageConfigHelpers)
install(TARGETS msft_proxy
        EXPORT Microsoft.ProxyConfig)
install(FILES proxy.h
        DESTINATION include)
export(EXPORT Microsoft.ProxyConfig FILE ${CMAKE_CURRENT_BINARY_DIR}/Microsoft.ProxyConfig.cmake)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/Microsoft.ProxyConfigVersion.cmake
                                 VERSION 1.0.0
                                 COMPATIBILITY AnyNewerVersion)
set(MSFT_PROXY_CMAKECONFIG_INSTALL_DIR "share/cmake/Microsoft.Proxy" CACHE STRING "install path for Microsoft.ProxyConfig.cmake")
install(EXPORT Microsoft.ProxyConfig
        FILE Microsoft.ProxyConfig.cmake
        DESTINATION ${MSFT_PROXY_CMAKECONFIG_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Microsoft.ProxyConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/Microsoft.ProxyConfigVersion.cmake
              DESTINATION ${MSFT_PROXY_CMAKECONFIG_INSTALL_DIR})

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
