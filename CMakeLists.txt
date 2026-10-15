cmake_minimum_required(VERSION 3.16)
project(
  project-study-graph
  VERSION 1.0.0
  LANGUAGES C CXX
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_subdirectory(beltgraph)
add_subdirectory(grapher)
add_subdirectory(tests)
