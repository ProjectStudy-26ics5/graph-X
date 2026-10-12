cmake_minimum_required(VERSION 3.16)
project(project-study-graph LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_subdirectory(beltgraph)
add_subdirectory(grapher)
add_subdirectory(tests)
