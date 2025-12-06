if(NOT DEFINED msft_proxy4_INCLUDE_DIR)
    message(FATAL_ERROR "`msft_proxy4_INCLUDE_DIR` must be defined to use this script.")
endif()

message(STATUS "Declaring `msft_proxy4::proxy_module` target for include path `${msft_proxy4_INCLUDE_DIR}`")

add_library(msft_proxy4_module)
set_target_properties(
  msft_proxy4_module
  PROPERTIES
    SYSTEM TRUE
    EXCLUDE_FROM_ALL TRUE
)

add_library(msft_proxy4::proxy_module ALIAS msft_proxy4_module)
target_sources(msft_proxy4_module PUBLIC
  FILE_SET CXX_MODULES
  BASE_DIRS ${msft_proxy4_INCLUDE_DIR}
  FILES
    ${msft_proxy4_INCLUDE_DIR}/proxy/v4/proxy.ixx
)
target_compile_features(msft_proxy4_module PUBLIC cxx_std_20)
target_compile_options(msft_proxy4_module PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
  $<$<CXX_COMPILER_ID:Clang>:-Wno-c++2b-extensions>
)
target_link_libraries(msft_proxy4_module PUBLIC msft_proxy4::proxy)
