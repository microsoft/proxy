project(msft_proxy_docs)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(EXTRACTION_SCRIPT ${CMAKE_SOURCE_DIR}/tools/extract_example_code_from_docs.py)
set(EXAMPLES_DIR ${CMAKE_BINARY_DIR}/examples_from_docs)
file(MAKE_DIRECTORY "${EXAMPLES_DIR}")
execute_process(
  COMMAND ${Python3_EXECUTABLE} ${EXTRACTION_SCRIPT} ${CMAKE_CURRENT_SOURCE_DIR} ${EXAMPLES_DIR}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND_ERROR_IS_FATAL ANY
)

add_subdirectory(
  ${EXAMPLES_DIR}
  ${EXAMPLES_DIR}/build
)