cmake_minimum_required(VERSION 3.10)

project(msft_proxy VERSION 3.4.0 LANGUAGES CXX)
add_library(msft_proxy INTERFACE)

# Do not enable building tests if proxy is consumed as
# subdirectory (e.g. by CMake FetchContent_Declare).
if(PROJECT_IS_TOP_LEVEL)
  option(BUILD_TESTING "Build tests" ON)
else()
  option(BUILD_TESTING "Build tests" OFF)
endif()

target_sources(msft_proxy
  INTERFACE
    FILE_SET public_headers
    TYPE HEADERS 
    BASE_DIRS include
    FILES
      include/proxy/proxy.h
)

target_compile_features(msft_proxy INTERFACE cxx_std_20)
target_include_directories(msft_proxy INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                                                $<INSTALL_INTERFACE:include>)

set(proxy_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
if(NOT PROJECT_IS_TOP_LEVEL)
  set(proxy_INCLUDE_DIR "${proxy_INCLUDE_DIR}" PARENT_SCOPE)
endif()

# install and export the project. project name - proxy
include(GNUInstallDirs)
install(TARGETS msft_proxy
        EXPORT proxyConfig
        FILE_SET public_headers DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(EXPORT proxyConfig DESTINATION ${CMAKE_INSTALL_DATADIR}/proxy)
export(TARGETS msft_proxy FILE proxyConfig.cmake)
include(CMakePackageConfigHelpers)
write_basic_package_version_file(proxyConfigVersion.cmake
                                 COMPATIBILITY SameMajorVersion
                                 ARCH_INDEPENDENT)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/proxyConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_DATADIR}/proxy)

# build tests if BUILD_TESTING is ON
if (BUILD_TESTING)
  include(CTest)
  add_subdirectory(tests)
  add_subdirectory(benchmarks)
  add_subdirectory(docs)
endif()
