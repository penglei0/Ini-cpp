cmake_minimum_required(VERSION 3.27)

project(
  Ini-cpp
  VERSION 1.0.0
  LANGUAGES CXX)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(
    FATAL_ERROR
      "
    FATAL: In-source builds are not allowed.
           You should create a separate directory for build files.
           ")
endif()

# Generating a compilation database: compile_commands.json
set(CMAKE_VERBOSE_MAKEFILE false)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

message(STATUS "CMAKE Version: ${CMAKE_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "System name: ${CMAKE_SYSTEM_NAME}")
message(STATUS "System version: ${CMAKE_SYSTEM_VERSION}")
message(STATUS "Host System name: ${CMAKE_HOST_SYSTEM_NAME}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

# Set build-specific compiler flags
set(CMAKE_NO_WARN_LIST " ")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -g -O3")
set(CMAKE_CXX_FLAGS_RELEASE
    "${CMAKE_CXX_FLAGS_RELEASE}  -Wall -Wextra -Wshadow -Werror ${CMAKE_NO_WARN_LIST} -O3"
)

# AddressSanitizer (ASan) is a runtime memory error detector for C/C++
option(ASAN_BUILD "Enable AddressSanitizer" OFF)
if(ASAN_BUILD)
  message(STATUS "Building with ASan enabled")
  add_compile_options(
    "$<$<COMPILE_LANGUAGE:CXX>:-fsanitize=address>"
    "$<$<COMPILE_LANGUAGE:CXX>:-O1>"
    "$<$<COMPILE_LANGUAGE:CXX>:-fno-omit-frame-pointer>")

  add_link_options("$<$<COMPILE_LANGUAGE:CXX>:-fsanitize=address>")
  add_link_options("$<$<COMPILE_LANGUAGE:CXX>:-static-liblsan>")
  add_link_options("$<$<COMPILE_LANGUAGE:CXX>:-no-pie>")
else()
  message(STATUS "Building without ASan")
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-s -z now -pie")
  endif()
endif()

option(BUILD_SHARED_LIBS "Build static libraries for all dependencies" OFF)

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
  FIND_PACKAGE_ARGS)
set(BUILD_TESTING
    OFF
    CACHE BOOL "" FORCE)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt
    ON
    CACHE BOOL "" FORCE)
set(gmock_build_tests
    OFF
    CACHE BOOL "" FORCE)
set(gtest_build_tests
    OFF
    CACHE BOOL "" FORCE)
set(INSTALL_GTEST
    OFF
    CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
message("GoogleTest source directory is :" ${googletest_SOURCE_DIR})
include_directories(${googletest_SOURCE_DIR}/googletest/include)

include(GoogleTest)
enable_testing()
add_custom_target(ini_all_test ${CMAKE_CTEST_COMMAND} -V)
set(CTEST_OUTPUT_ON_FAILURE true)
set(GTEST_COLOR true)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(Ini-cpp INTERFACE)
target_include_directories(
  Ini-cpp INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                    $<INSTALL_INTERFACE:include>)
target_compile_features(Ini-cpp INTERFACE cxx_std_17)
install(
  TARGETS Ini-cpp
  EXPORT Ini-cpp
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION include)

# ini_settings_test
add_executable(ini_settings_test test/ini_settings_test.cc)
target_link_libraries(ini_settings_test gtest_main gmock_main)
gtest_discover_tests(ini_settings_test)

# Add the test command with custom arguments add_test(NAME ini_settings_test
# COMMAND ini_settings_test ${CMAKE_SOURCE_DIR}/test/)

add_executable(ini_util_func_test test/ini_util_func_test.cc)
target_link_libraries(ini_util_func_test gtest_main gmock_main)
gtest_discover_tests(ini_util_func_test)
