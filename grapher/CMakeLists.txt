add_library(grapher)
target_compile_features(grapher PUBLIC cxx_std_17)

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
  "include"
  ${OSMIUM_INCLUDE_DIRS}
)

target_link_libraries(
  grapher
  PUBLIC
  beltgraph
  PRIVATE
  ${OSMIUM_LIBRARIES}
)
