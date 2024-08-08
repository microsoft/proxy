project(msft_proxy_samples)

file(GLOB_RECURSE SOURCES "*.cpp")

foreach(SOURCE ${SOURCES})
  file(RELATIVE_PATH REL_PATH ${CMAKE_SOURCE_DIR} ${SOURCE})
  get_filename_component(DIR ${REL_PATH} DIRECTORY)
  string(REPLACE "/" "_" DIR_UNDERSCORE ${DIR})
  get_filename_component(EXECUTABLE_NAME ${SOURCE} NAME_WE)
  set(FULL_EXECUTABLE_NAME "${DIR_UNDERSCORE}_${EXECUTABLE_NAME}")
  add_executable(${FULL_EXECUTABLE_NAME} ${SOURCE})
  target_link_libraries(${FULL_EXECUTABLE_NAME} PRIVATE msft_proxy)
endforeach()

if (MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()
