cmake_minimum_required(VERSION 3.15)
project(RoaringComparison LANGUAGES CXX C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Disable tests for subprojects
set(ENABLE_ROARING_TESTS OFF CACHE BOOL "Disable roaring tests" FORCE)

# Build v1 library
message(STATUS "Configuring Roaring v1...")
set(ROARING_BUILD_STATIC ON CACHE BOOL "Build v1 as static" FORCE)

# Patch v1 CMakeLists.txt to use roaring_v1 as library name
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/v1/CMakeLists.txt" V1_CMAKE_CONTENT)
string(REGEX REPLACE "set\\(ROARING_LIB_NAME roaring\\)" "set(ROARING_LIB_NAME roaring_v1)" V1_CMAKE_CONTENT "${V1_CMAKE_CONTENT}")
# Backup and write patched v1 CMakeLists.txt
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/v1/CMakeLists.txt.original")
    file(RENAME "${CMAKE_CURRENT_SOURCE_DIR}/v1/CMakeLists.txt" "${CMAKE_CURRENT_SOURCE_DIR}/v1/CMakeLists.txt.original")
endif()
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/v1/CMakeLists.txt" "${V1_CMAKE_CONTENT}")

add_subdirectory(v1 ${CMAKE_BINARY_DIR}/v1_build)

# Clear cache variables for v2
unset(ROARING_BUILD_STATIC CACHE)
unset(ROARING_LIB_NAME CACHE)

# Build v2 library - need to patch it to use different target names
message(STATUS "Configuring Roaring v2...")
set(ROARING_BUILD_STATIC ON CACHE BOOL "Build v2 as static" FORCE)

# Patch v2 main CMakeLists.txt to use different header target names
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/v2/CMakeLists.txt" V2_MAIN_CMAKE_CONTENT)
# Replace in order: most specific patterns first to avoid double replacement
# 1. Replace add_library declarations
string(REGEX REPLACE "add_library\\(roaring-headers-cpp " "add_library(roaring-headers-cpp-v2 " V2_MAIN_CMAKE_CONTENT "${V2_MAIN_CMAKE_CONTENT}")
string(REGEX REPLACE "add_library\\(roaring-headers " "add_library(roaring-headers-v2 " V2_MAIN_CMAKE_CONTENT "${V2_MAIN_CMAKE_CONTENT}")
# 2. Replace target_include_directories
string(REGEX REPLACE "target_include_directories\\(roaring-headers-cpp " "target_include_directories(roaring-headers-cpp-v2 " V2_MAIN_CMAKE_CONTENT "${V2_MAIN_CMAKE_CONTENT}")
string(REGEX REPLACE "target_include_directories\\(roaring-headers " "target_include_directories(roaring-headers-v2 " V2_MAIN_CMAKE_CONTENT "${V2_MAIN_CMAKE_CONTENT}")
# 3. Replace install TARGETS (this line has both targets)
string(REGEX REPLACE "install\\(TARGETS roaring-headers roaring-headers-cpp" "install(TARGETS roaring-headers-v2 roaring-headers-cpp-v2" V2_MAIN_CMAKE_CONTENT "${V2_MAIN_CMAKE_CONTENT}")
# 4. Replace EXPORT name
string(REGEX REPLACE "EXPORT roaring-config" "EXPORT roaring_v2-config" V2_MAIN_CMAKE_CONTENT "${V2_MAIN_CMAKE_CONTENT}")
# 5. Replace ALIAS (roaring::roaring ALIAS roaring -> roaring::roaring_v2 ALIAS roaring_v2)
string(REGEX REPLACE "add_library\\(roaring::roaring ALIAS roaring\\)" "add_library(roaring::roaring_v2 ALIAS roaring_v2)" V2_MAIN_CMAKE_CONTENT "${V2_MAIN_CMAKE_CONTENT}")
# 6. Replace set_target_properties for roaring (must be after ALIAS to avoid double replacement)
string(REGEX REPLACE "set_target_properties\\([\n\t ]*roaring " "set_target_properties(\n  roaring_v2 " V2_MAIN_CMAKE_CONTENT "${V2_MAIN_CMAKE_CONTENT}")

# Backup and write patched main CMakeLists.txt
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/v2/CMakeLists.txt.original")
    file(RENAME "${CMAKE_CURRENT_SOURCE_DIR}/v2/CMakeLists.txt" "${CMAKE_CURRENT_SOURCE_DIR}/v2/CMakeLists.txt.original")
endif()
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/v2/CMakeLists.txt" "${V2_MAIN_CMAKE_CONTENT}")

# Patch v2/src/CMakeLists.txt to use roaring_v2 as target name and updated header names
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/v2/src/CMakeLists.txt" V2_SRC_CMAKE_CONTENT)
# First replace header references (more specific patterns first)
string(REGEX REPLACE "roaring-headers-cpp([^-])" "roaring-headers-cpp-v2\\1" V2_SRC_CMAKE_CONTENT "${V2_SRC_CMAKE_CONTENT}")
string(REGEX REPLACE "roaring-headers([^-])" "roaring-headers-v2\\1" V2_SRC_CMAKE_CONTENT "${V2_SRC_CMAKE_CONTENT}")
# Then replace roaring target (use word boundary to avoid replacing roaring_v2)
string(REGEX REPLACE "add_library\\(roaring " "add_library(roaring_v2 " V2_SRC_CMAKE_CONTENT "${V2_SRC_CMAKE_CONTENT}")
string(REGEX REPLACE "target_compile_definitions\\(roaring " "target_compile_definitions(roaring_v2 " V2_SRC_CMAKE_CONTENT "${V2_SRC_CMAKE_CONTENT}")
string(REGEX REPLACE "target_link_libraries\\(roaring " "target_link_libraries(roaring_v2 " V2_SRC_CMAKE_CONTENT "${V2_SRC_CMAKE_CONTENT}")
string(REGEX REPLACE "install\\(TARGETS roaring " "install(TARGETS roaring_v2 " V2_SRC_CMAKE_CONTENT "${V2_SRC_CMAKE_CONTENT}")
string(REGEX REPLACE "set_target_properties\\(roaring([^_])" "set_target_properties(roaring_v2\\1" V2_SRC_CMAKE_CONTENT "${V2_SRC_CMAKE_CONTENT}")
string(REGEX REPLACE "target_compile_options\\(roaring " "target_compile_options(roaring_v2 " V2_SRC_CMAKE_CONTENT "${V2_SRC_CMAKE_CONTENT}")
# Replace export and config names
string(REGEX REPLACE "EXPORT roaring-config" "EXPORT roaring_v2-config" V2_SRC_CMAKE_CONTENT "${V2_SRC_CMAKE_CONTENT}")
string(REGEX REPLACE "export\\(EXPORT roaring-config" "export(EXPORT roaring_v2-config" V2_SRC_CMAKE_CONTENT "${V2_SRC_CMAKE_CONTENT}")
string(REGEX REPLACE "install\\(EXPORT roaring-config" "install(EXPORT roaring_v2-config" V2_SRC_CMAKE_CONTENT "${V2_SRC_CMAKE_CONTENT}")
string(REGEX REPLACE "roaring-targets\\.cmake" "roaring_v2-targets.cmake" V2_SRC_CMAKE_CONTENT "${V2_SRC_CMAKE_CONTENT}")
string(REGEX REPLACE "roaring-config\\.cmake" "roaring_v2-config.cmake" V2_SRC_CMAKE_CONTENT "${V2_SRC_CMAKE_CONTENT}")
string(REGEX REPLACE "roaring-config-version\\.cmake" "roaring_v2-config-version.cmake" V2_SRC_CMAKE_CONTENT "${V2_SRC_CMAKE_CONTENT}")

# Backup and write patched src/CMakeLists.txt
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/v2/src/CMakeLists.txt.original")
    file(RENAME "${CMAKE_CURRENT_SOURCE_DIR}/v2/src/CMakeLists.txt" "${CMAKE_CURRENT_SOURCE_DIR}/v2/src/CMakeLists.txt.original")
endif()
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/v2/src/CMakeLists.txt" "${V2_SRC_CMAKE_CONTENT}")

add_subdirectory(v2 ${CMAKE_BINARY_DIR}/v2_build)

# Create test helper source files
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/test_v1.cpp
"#include <iostream>
#include <cstdint>
#include \"roaring64map.hh\"

extern \"C\" size_t test_roaring_v1(const uint64_t* values, size_t count, bool portable) {
    roaring::Roaring64Map bitmap;
    for (size_t i = 0; i < count; ++i) {
        bitmap.add(values[i]);
    }
    return bitmap.getSizeInBytes(portable);
}
")

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/test_v2.cpp
"#include <iostream>
#include <cstdint>
#include \"roaring/roaring64map.hh\"

extern \"C\" size_t test_roaring_v2(const uint64_t* values, size_t count, bool portable) {
    roaring::Roaring64Map bitmap;
    for (size_t i = 0; i < count; ++i) {
        bitmap.add(values[i]);
    }
    return bitmap.getSizeInBytes(portable);
}
")

# Build v1 test library
add_library(test_v1_lib STATIC ${CMAKE_CURRENT_BINARY_DIR}/test_v1.cpp)
target_include_directories(test_v1_lib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/v1/include
    ${CMAKE_CURRENT_SOURCE_DIR}/v1/cpp
)
target_link_libraries(test_v1_lib PRIVATE roaring_v1)
set_target_properties(test_v1_lib PROPERTIES OUTPUT_NAME test_roaring_v1)

# Build v2 test library
add_library(test_v2_lib STATIC ${CMAKE_CURRENT_BINARY_DIR}/test_v2.cpp)
target_include_directories(test_v2_lib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/v2/include
    ${CMAKE_CURRENT_SOURCE_DIR}/v2/cpp
)
target_link_libraries(test_v2_lib PRIVATE roaring_v2)
set_target_properties(test_v2_lib PROPERTIES OUTPUT_NAME test_roaring_v2)

# Build main executable
add_executable(main main.cpp)
target_link_libraries(main PRIVATE test_v1_lib test_v2_lib)
# Allow multiple definitions because v1 and v2 have the same C symbols
# The test libraries encapsulate the roaring libraries, so there's no conflict in practice
if(NOT MSVC)
    target_link_options(main PRIVATE "-Wl,--allow-multiple-definition")
endif()

message(STATUS "=== Configuration Summary ===")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "=============================")
