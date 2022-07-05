cmake_minimum_required(VERSION 3.14)
find_package(proxy CONFIG REQUIRED)
add_executable(demo
               demo.cpp)
target_link_libraries(demo PRIVATE msft_proxy)
