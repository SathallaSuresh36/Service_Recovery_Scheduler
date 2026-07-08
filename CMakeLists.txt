cmake_minimum_required(VERSION 3.14)
project(service_recovery_scheduler CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# use C++20 if we can (for concepts), otherwise fall back to C++17
if("cxx_std_20" IN_LIST CMAKE_CXX_COMPILE_FEATURES)
    set(cxx_standard 20)
else()
    set(cxx_standard 17)
endif()
message(STATUS "building with C++${cxx_standard}")

# optional code-coverage instrumentation (GCC/Clang only), off by default
option(SRS_ENABLE_COVERAGE "Build with coverage instrumentation (gcov)" OFF)

# the library
add_library(srs
    src/actions.cpp
    src/service.cpp
    src/scheduler.cpp
    src/recovery_factory.cpp
    src/logger.cpp)
target_include_directories(srs PUBLIC include)
target_compile_features(srs PUBLIC cxx_std_${cxx_standard})

# extra warnings to keep the build clean; the flag syntax differs per compiler
if(MSVC)
    target_compile_options(srs PRIVATE /W4)
else()
    target_compile_options(srs PRIVATE -Wall -Wextra)
endif()

if(SRS_ENABLE_COVERAGE AND NOT MSVC)
    # instrument the library so its lines are counted when the tests run
    target_compile_options(srs PUBLIC --coverage -O0 -g)
    target_link_options(srs PUBLIC --coverage)
endif()

# demo program
add_executable(srs_demo src/main.cpp)
target_link_libraries(srs_demo PRIVATE srs)

# tests
enable_testing()
add_subdirectory(unit_tests)
