cmake_minimum_required(VERSION 3.10)

project(reaction VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0  # ✅ 正确版本
)

# Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

if(MSVC)
    add_compile_options(/W4 /GR-)
else()
    add_compile_options(-Wall -Wextra -pedantic -fno-rtti -std=c++2a)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

enable_testing()
file(GLOB TEST_SOURCES ${PROJECT_SOURCE_DIR}/test/*.cpp)
add_executable(runTests ${TEST_SOURCES})

target_link_libraries(runTests
    PRIVATE
        GTest::gtest
        GTest::gtest_main
        ${PROJECT_NAME}
)

include(GoogleTest)
gtest_discover_tests(runTests)