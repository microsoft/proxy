cmake_minimum_required(VERSION 3.28)

project(msft_proxy VERSION 3.3.0 LANGUAGES CXX)
add_library(msft_proxy)

target_sources(msft_proxy
  PUBLIC
    FILE_SET CXX_MODULES
    TYPE CXX_MODULES 
    BASE_DIRS include
    FILES
      include/proxy/proxy.cppm
      include/proxy/proxy_interface.cppm
)

target_sources(msft_proxy
  PUBLIC
    FILE_SET public_headers 
    TYPE HEADERS 
    BASE_DIRS include
    FILES
      include/proxy/proxy.h
      include/proxy/proxy_macros.h
      include/proxy/proxy_fmt.h
      include/proxy/details/proxy_impl.h
      include/proxy/details/proxy_impl_macros.h
)

target_compile_features(msft_proxy PUBLIC cxx_std_20)
target_include_directories(msft_proxy PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                                                $<INSTALL_INTERFACE:include>)

# install and export the project. project name - proxy
include(GNUInstallDirs)
install(TARGETS msft_proxy
        EXPORT proxyConfig
        FILE_SET CXX_MODULES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
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
include(CTest)
if (BUILD_TESTING)
  add_subdirectory(tests)
  add_subdirectory(benchmarks)
  add_subdirectory(docs)
endif()
