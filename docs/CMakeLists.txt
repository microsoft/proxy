project(msft_proxy_docs)

FetchContent_Declare(
  fmt
  URL https://github.com/fmtlib/fmt/archive/refs/tags/11.2.0.tar.gz
  URL_HASH SHA256=ac366b7b4c2e9f0dde63a59b3feb5ee59b67974b14ee5dc9ea8ad78aa2c1ee1e
)
FetchContent_MakeAvailable(fmt)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

file(GLOB_RECURSE DOC_FILES "*.md")
set(EXTRACTION_SCRIPT ${CMAKE_SOURCE_DIR}/tools/extract_example_code_from_docs.py)
set(EXAMPLES_DIR ${CMAKE_BINARY_DIR}/examples_from_docs)
file(MAKE_DIRECTORY "${EXAMPLES_DIR}")
execute_process(
  COMMAND ${Python3_EXECUTABLE} ${EXTRACTION_SCRIPT} ${CMAKE_CURRENT_SOURCE_DIR} ${EXAMPLES_DIR}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND_ERROR_IS_FATAL ANY
)

file(GLOB EXAMPLE_SOURCES "${EXAMPLES_DIR}/*.cpp")
set_source_files_properties(${EXAMPLE_SOURCES} PROPERTIES GENERATED TRUE)
foreach(SOURCE ${EXAMPLE_SOURCES})
  get_filename_component(EXECUTABLE_NAME ${SOURCE} NAME_WE)
  add_executable(${EXECUTABLE_NAME} ${SOURCE})
  target_link_libraries(${EXECUTABLE_NAME} PRIVATE msft_proxy)
  if (MSVC)
    target_compile_options(${EXECUTABLE_NAME} PRIVATE /W4)
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${EXECUTABLE_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wno-c++2b-extensions)
  else()
    target_compile_options(${EXECUTABLE_NAME} PRIVATE -Wall -Wextra -Wpedantic)
  endif()
endforeach()

target_link_libraries(example_spec_skills_fmt_format PRIVATE fmt::fmt)
