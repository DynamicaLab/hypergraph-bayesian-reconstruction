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

if(SKBUILD)
    # Scikit-Build does not add your site-packages to the search path
    # automatically, so we need to add it _or_ the pybind11 specific directory
    # here.
    execute_process(
        COMMAND "${PYTHON_EXECUTABLE}" -c
            "import pybind11; print(pybind11.get_cmake_dir())"
        OUTPUT_VARIABLE _tmp_dir
        OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ECHO STDOUT
    )
    list(APPEND CMAKE_PREFIX_PATH "${_tmp_dir}")

    find_package(pybind11 REQUIRED)


    file(GLOB_RECURSE BINDING_SRC "${PROJECT_SOURCE_DIR}/python_wrapper/*.cpp")
    file(GLOB_RECURSE INFERENCE_MODELS_SRC "${PROJECT_SOURCE_DIR}/src/inference-models/*.cpp")

    pybind11_add_module(pygrit MODULE ${BINDING_SRC} ${INFERENCE_MODELS_SRC})

    target_link_libraries(pygrit PRIVATE GRIT)
    set_target_properties(pygrit PROPERTIES LIBRARY_OUTPUT_DIRECTORY ./pygrit)
    install(TARGETS pygrit DESTINATION ./pygrit)
endif()


if (BUILD_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)
    include_directories(${GTEST_INCLUDE_DIRS})

    add_subdirectory(tests)
endif()
