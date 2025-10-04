cmake_minimum_required(VERSION 3.28)

project(msft_proxy4 VERSION 4.0.1 LANGUAGES CXX)
add_library(msft_proxy4 INTERFACE)
set_target_properties(msft_proxy4 PROPERTIES EXPORT_NAME proxy)
add_library(msft_proxy4::proxy ALIAS msft_proxy4)

# Do not enable building tests if proxy is consumed as
# subdirectory (e.g. by CMake FetchContent_Declare).
if(PROJECT_IS_TOP_LEVEL)
  option(BUILD_TESTING "Build tests" ON)
else()
  option(BUILD_TESTING "Build tests" OFF)
endif()

if(PROJECT_IS_TOP_LEVEL)
  option(
    PROXY_BUILD_MODULES
    "When this project is top level, build the docs and tests with C++ module support."
    OFF
  )
endif()

target_sources(msft_proxy4
  INTERFACE
    FILE_SET public_headers
    TYPE HEADERS 
    BASE_DIRS include
    FILES
      include/proxy/proxy.h
      include/proxy/proxy_macros.h
      include/proxy/proxy_fmt.h
      include/proxy/v4/proxy.ixx
      include/proxy/v4/proxy.h
      include/proxy/v4/proxy_macros.h
      include/proxy/v4/proxy_fmt.h
)

target_compile_features(msft_proxy4 INTERFACE cxx_std_20)
target_include_directories(msft_proxy4 INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                                                $<INSTALL_INTERFACE:include>)

# Do not set the module target if proxy is consumed as a subdirectory.
if(PROJECT_IS_TOP_LEVEL)
  set(msft_proxy4_INCLUDE_DIR include)
  if(PROXY_BUILD_MODULES)
    include(cmake/msft_proxy4ModuleTargets.cmake)
  endif()
else()
  # Propagate the variable to the parent project
  set(msft_proxy4_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include PARENT_SCOPE)
endif()

# install and export the project. project name - proxy

include(GNUInstallDirs)

install(TARGETS msft_proxy4
        EXPORT msft_proxy4Targets
        FILE_SET public_headers DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
  EXPORT msft_proxy4Targets
  NAMESPACE msft_proxy4::
  DESTINATION ${CMAKE_INSTALL_DATADIR}/msft_proxy4
)
export(
  TARGETS msft_proxy4
  NAMESPACE msft_proxy4::
  FILE msft_proxy4Targets.cmake
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
  cmake/msft_proxy4Config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/msft_proxy4Config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}/msft_proxy4
  PATH_VARS CMAKE_INSTALL_INCLUDEDIR
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(cmake/msft_proxy4ConfigVersion.cmake
                                 COMPATIBILITY SameMajorVersion
                                 ARCH_INDEPENDENT)

install(
  FILES 
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/msft_proxy4Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/msft_proxy4ConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_DATADIR}/msft_proxy4
)

# build tests if BUILD_TESTING is ON
if (BUILD_TESTING)
  include(CTest)
  add_subdirectory(tests)
  add_subdirectory(benchmarks)
  add_subdirectory(docs)
endif()
