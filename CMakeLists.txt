cmake_minimum_required (VERSION 3.10)

enable_testing ()

set (CMAKE_CXX_STANDARD 17)

project (PROFILER)

set (LIBELFIN_DIR "${PROJECT_SOURCE_DIR}/../../software/libelfin/libelfin-v1")

include_directories ("${PROJECT_SOURCE_DIR}/../../software/boost/boost-1-61-0")

set (CMAKE_CXX_FLAGS "-Werror -Wall -g -fpic")
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

##########################################################################
# Core library
##########################################################################

include_directories (src)

file (
  GLOB LIBRARY_SOURCES "src/profile/*.cpp"
)

add_library (
  libprofile SHARED ${LIBRARY_SOURCES}
)

add_library (
  libelf++ SHARED IMPORTED
)

set_target_properties (
  libelf++ PROPERTIES IMPORTED_LOCATION
  "${LIBELFIN_DIR}/elf/libelf++.so"
)

add_library (
  libdwarf++ SHARED IMPORTED
)

set_target_properties (
  libdwarf++ PROPERTIES IMPORTED_LOCATION
  "${LIBELFIN_DIR}/dwarf/libdwarf++.so"
)

target_include_directories (
  libprofile PUBLIC
  "${LIBELFIN_DIR}/elf"
  "${LIBELFIN_DIR}/dwarf")

##########################################################################
# Commandline tool
##########################################################################

add_executable (
  tool tools/profile/profile.cpp
)

target_link_libraries (
  tool libprofile libelf++ libdwarf++
)

##########################################################################
# Unit test executable
##########################################################################

file (
  GLOB UNIT_TEST_SOURCES "tests/test_profiler/*.cpp"
)

add_executable (
  test_profiler
  ${UNIT_TEST_SOURCES}
)

set_target_properties (
  test_profiler
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tests
  COMPILE_FLAGS "-Wno-placement-new"
)

target_link_libraries (
  test_profiler libprofile libelf++ libdwarf++
)

##########################################################################
# Run unit test
##########################################################################

add_test (
  NAME test_profiler
  COMMAND test_profiler
)

##########################################################################
# Tool regression test
##########################################################################

file (
  GLOB
  REGRESSION_TEST_SOURCES
  "tests/test_tool_regression/test_executables/*.cpp"
)

foreach (file ${REGRESSION_TEST_SOURCES})
  get_filename_component (EXECUTABLE ${file} NAME_WE)
  add_executable (${EXECUTABLE} ${file})
  set_target_properties (
    ${EXECUTABLE}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tests
  )
endforeach ()

add_test (
  NAME test_tool_regression
  COMMAND
  ${CMAKE_COMMAND} -E env
  BIN_DIR=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  TEST_EXECUTABLES=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tests
  ${PROJECT_SOURCE_DIR}/tests/test_runner/test_runner.sh
  ${PROJECT_SOURCE_DIR}/tests/test_tool_regression/test_tool_regression.sh
  ${PROJECT_SOURCE_DIR}/tests/test_tool_regression/expected.log
)

##########################################################################
# Copyright test
##########################################################################


add_test (
  NAME test_copyright
  COMMAND
  ${CMAKE_COMMAND} -E env
  PROJECT_ROOT=${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/tests/test_copyright/test_copyright.sh
)
