
if(NOT DEFINED proxy_INCLUDE_DIR)
    message(FATAL_ERROR "proxy_INCLUDE_DIR must be defined to use this script.")
endif()

message(STATUS "Declaring `msft_proxy::module` target for include path ${proxy_INCLUDE_DIR}")

add_library( msft_proxy_module INTERFACE IMPORTED )
set_target_properties(
    msft_proxy_module 
    PROPERTIES
        SYSTEM TRUE
        EXCLUDE_FROM_ALL TRUE
)

add_library( msft_proxy::module ALIAS msft_proxy_module )
target_sources( msft_proxy_module INTERFACE
    FILE_SET CXX_MODULES
    BASE_DIRS ${proxy_INCLUDE_DIR}
    FILES
        ${proxy_INCLUDE_DIR}/proxy/proxy.cppm
        ${proxy_INCLUDE_DIR}/proxy/proxy_interface.cppm
)
target_compile_features( msft_proxy_module INTERFACE cxx_std_26 )
target_link_libraries( msft_proxy_module INTERFACE msft_proxy )

