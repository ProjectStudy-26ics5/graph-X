add_library(beltgraph STATIC)

target_sources(
  beltgraph
  PRIVATE
  "src/reader.c"
  "src/header.c"
  "src/way.c"
  "src/node.c"
)

target_include_directories(
  beltgraph
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_definitions(
  beltgraph
  PUBLIC
  GRAPHER
)

add_library(PS::beltgraph ALIAS beltgraph)

install(
  TARGETS beltgraph
  EXPORT beltgraphTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
  DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

set(beltgraph_INSTALL_CMAKEDIR
  "${CMAKE_INSTALL_LIBDIR}/cmake/beltgraph"
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/beltgraphConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/beltgraphConfig.cmake"
  INSTALL_DESTINATION "${beltgraph_INSTALL_CMAKEDIR}"
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/beltgraphConfigVersion.cmake"
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY SameMajorVersion
)

install(
  EXPORT beltgraphTargets
  FILE beltgraphTargets.cmake
  NAMESPACE PS::
  DESTINATION "${beltgraph_INSTALL_CMAKEDIR}"
)

install(
  FILES
  "${CMAKE_CURRENT_BINARY_DIR}/beltgraphConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/beltgraphConfigVersion.cmake"
  DESTINATION "${beltgraph_INSTALL_CMAKEDIR}"
)
