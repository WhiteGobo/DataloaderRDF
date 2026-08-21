cmake_minimum_required(VERSION 3.25)

project(DataloaderRDF VERSION 0.1.0 LANGUAGES C)

#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(packages.cmake)

add_subdirectory(src)

enable_testing()

add_subdirectory(test)
