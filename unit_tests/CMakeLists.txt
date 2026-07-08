include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/release-1.10.0.tar.gz)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)


if(CYGWIN)
    set(gtest_disable_pthreads ON CACHE BOOL "" FORCE)
endif()

FetchContent_MakeAvailable(googletest)

# Keep GoogleTest at the same C++ standard as the tests for a consistent ABI.
foreach(gtest_target gtest gtest_main gmock gmock_main)
    if(TARGET ${gtest_target})
        set_target_properties(${gtest_target} PROPERTIES CXX_STANDARD 17)
        if(CYGWIN)
            set_target_properties(${gtest_target} PROPERTIES CXX_EXTENSIONS ON)
        endif()
    endif()
endforeach()

add_executable(srs_tests
    test_actions.cpp
    test_service.cpp
    test_scheduler.cpp
    test_factory.cpp
    test_logger.cpp)

target_link_libraries(srs_tests PRIVATE
    srs
    gmock_main)

target_include_directories(srs_tests SYSTEM PRIVATE
    ${googletest_SOURCE_DIR}/googletest/include
    ${googletest_SOURCE_DIR}/googlemock/include)


if(CYGWIN)
    set_target_properties(srs_tests PROPERTIES CXX_EXTENSIONS ON)
endif()

include(GoogleTest)
gtest_discover_tests(srs_tests)
