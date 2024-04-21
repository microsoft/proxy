cmake_minimum_required (VERSION 3.5)

project(msft_proxy
        VERSION 0.1.0 # local build version
        LANGUAGES CXX)
add_library(msft_proxy INTERFACE)
target_compile_features(msft_proxy INTERFACE cxx_std_20)
target_include_directories(msft_proxy INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                                                $<INSTALL_INTERFACE:include>)

# install and export the project. project name - proxy
include(GNUInstallDirs)
install(TARGETS msft_proxy
        EXPORT proxyConfig)
install(FILES proxy.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/proxy)
install(EXPORT proxyConfig DESTINATION ${CMAKE_INSTALL_DATADIR}/proxy)
export(TARGETS msft_proxy FILE proxyConfig.cmake)
include(CMakePackageConfigHelpers)
write_basic_package_version_file(proxyConfigVersion.cmake
                                 COMPATIBILITY SameMajorVersion
                                 ARCH_INDEPENDENT)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/proxyConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_DATADIR}/proxy)

# build tests if BUILD_TESTING is ON
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
