add_library(grapher STATIC)
target_compile_features(grapher PUBLIC cxx_std_17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

target_sources(
  grapher
  PRIVATE
  "src/filter.cpp"
  "src/way.cpp"
  "src/node.cpp"
  "src/graph.cpp"
)

find_package(Osmium REQUIRED COMPONENTS io)

target_include_directories(
  grapher
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${OSMIUM_INCLUDE_DIRS}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(
  grapher
  PUBLIC
  PS::beltgraph
  PRIVATE
  ${OSMIUM_LIBRARIES}
)

add_library(PS::grapher ALIAS grapher)

install(
  TARGETS grapher
  EXPORT grapherTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
  DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

set(grapher_INSTALL_CMAKEDIR
  "${CMAKE_INSTALL_LIBDIR}/cmake/grapher"
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/grapherConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/grapherConfig.cmake"
  INSTALL_DESTINATION "${grapher_INSTALL_CMAKEDIR}"
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/grapherConfigVersion.cmake"
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY SameMajorVersion
)

install(
  EXPORT grapherTargets
  FILE grapherTargets.cmake
  NAMESPACE PS::
  DESTINATION "${grapher_INSTALL_CMAKEDIR}"
)

install(
  FILES
  "${CMAKE_CURRENT_BINARY_DIR}/grapherConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/grapherConfigVersion.cmake"
  DESTINATION "${grapher_INSTALL_CMAKEDIR}"
)
