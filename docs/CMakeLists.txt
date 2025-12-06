project(msft_proxy_docs)

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
  target_link_libraries(${EXECUTABLE_NAME} PRIVATE msft_proxy4::proxy)
  if (MSVC)
    target_compile_options(${EXECUTABLE_NAME} PRIVATE /W4)
  else()
    target_compile_options(${EXECUTABLE_NAME} PRIVATE -Wall -Wextra -Wpedantic $<$<CXX_COMPILER_ID:Clang>:-Wno-c++2b-extensions>)
  endif()
endforeach()

target_link_libraries(example_spec_skills_fmt_format PRIVATE fmt::fmt)
