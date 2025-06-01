
if(NOT DEFINED proxy4_INCLUDE_DIR)
    message(FATAL_ERROR "proxy4_INCLUDE_DIR must be defined to use this script.")
endif()

message(STATUS "Declaring `msft_proxy::msft_proxy4_module` target for include path ${proxy4_INCLUDE_DIR}")

add_library(msft_proxy4_module)
set_target_properties(
  msft_proxy4_module
  PROPERTIES
    SYSTEM TRUE
    EXCLUDE_FROM_ALL TRUE
)

add_library(msft_proxy::msft_proxy4_module ALIAS msft_proxy4_module)
target_sources(msft_proxy4_module PUBLIC
  FILE_SET CXX_MODULES
  BASE_DIRS ${proxy4_INCLUDE_DIR}
  FILES
    ${proxy4_INCLUDE_DIR}/proxy/proxy.ixx
)
target_compile_features(msft_proxy4_module PUBLIC cxx_std_20)
target_link_libraries(msft_proxy4_module PUBLIC msft_proxy::msft_proxy4)

