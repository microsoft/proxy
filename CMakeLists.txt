cmake_minimum_required(VERSION 3.28)

project(msft_proxy VERSION 3.3.0 LANGUAGES CXX)
add_library(msft_proxy INTERFACE)

# Do not enable building tests if proxy is consumed as
# subdirectory (e.g. by CMake FetchContent_Declare).
if(PROJECT_IS_TOP_LEVEL)
  option(BUILD_TESTING "Build tests" ON)
else()
  option(BUILD_TESTING "Build tests" OFF)
endif()

if(PROJECT_IS_TOP_LEVEL)
  option(
    PROXY_BUILDING_WITH_MODULE 
    "When this project is top level, build the docs and tests with C++ module support."
    ON
  )
endif()

target_sources(msft_proxy
  INTERFACE
    FILE_SET public_headers
    TYPE HEADERS 
    BASE_DIRS include
    FILES
      include/proxy/proxy.h
      include/proxy/proxy_macros.h
      include/proxy/proxy_fmt.h
      include/proxy/proxy.ixx
)

target_compile_features(msft_proxy INTERFACE cxx_std_20)
target_include_directories(msft_proxy INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                                                $<INSTALL_INTERFACE:include>)

# Do not set the module target if proxy is consumed as a subdirectory.
if(PROJECT_IS_TOP_LEVEL)
  set(proxy_INCLUDE_DIR include)
  include(cmake/proxyModuleTargets.cmake)
else()
  # Propagate the variable to the parent project
  set(proxy_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include PARENT_SCOPE)
endif()

# install and export the project. project name - proxy

include(GNUInstallDirs)

install(TARGETS msft_proxy
        EXPORT proxyTargets
        FILE_SET public_headers DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT proxyTargets DESTINATION ${CMAKE_INSTALL_DATADIR}/proxy)
export(TARGETS msft_proxy FILE proxyTargets.cmake)

include(CMakePackageConfigHelpers)
configure_package_config_file(
  cmake/proxyConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/proxyConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}/proxy
  PATH_VARS CMAKE_INSTALL_INCLUDEDIR
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(cmake/proxyConfigVersion.cmake
                                 COMPATIBILITY SameMajorVersion
                                 ARCH_INDEPENDENT)

install(
  FILES 
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/proxyConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/proxyConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_DATADIR}/proxy
)

# build tests if BUILD_TESTING is ON
if (BUILD_TESTING)
  include(CTest)
  add_subdirectory(tests)
  add_subdirectory(benchmarks)
  add_subdirectory(docs)
endif()
