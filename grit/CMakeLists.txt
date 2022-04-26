cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(NetworkInferenceOfTriangleCorrelation)
set(CMAKE_CXX_STANDARD 17)

option(BUILD_TESTS "build gtest unit tests" OFF)


find_package(Boost COMPONENTS filesystem REQUIRED)

get_filename_component(PARENT_DIR ${CMAKE_SOURCE_DIR} DIRECTORY)
include_directories(${CMAKE_SOURCE_DIR}/include/SamplableSet/src)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(include)

add_subdirectory(src)

if (BUILD_TESTS)
    set(CMAKE_CXX_STANDARD 17)
    enable_testing()
    find_package(GTest REQUIRED)
    include_directories(${GTEST_INCLUDE_DIRS})

    add_subdirectory(tests)
endif()
