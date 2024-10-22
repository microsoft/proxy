cmake_minimum_required(VERSION 3.5)

project(report_generator)

include(FetchContent)
# The policy uses the download time for timestamp, instead of the timestamp in the archive. This
# allows for proper rebuilds when a projects URL changes.
if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

FetchContent_Declare(
  nlohmann_json
  URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
  URL_HASH SHA256=d6c65aca6b1ed68e7a182f4757257b107ae403032760ed6ef121c9d55e81757d
)
FetchContent_MakeAvailable(nlohmann_json)

add_executable(report_generator main.cpp)

target_link_libraries(report_generator PRIVATE nlohmann_json::nlohmann_json)

target_compile_features(report_generator PRIVATE cxx_std_20)
if (MSVC)
  target_compile_options(report_generator PRIVATE /W4 /WX)
else()
  target_compile_options(report_generator PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
