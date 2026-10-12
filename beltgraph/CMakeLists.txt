add_library(beltgraph)

target_sources(
  beltgraph
  PRIVATE
  "src/header.c"
  "src/way.c"
  "src/node.c"
)

target_include_directories(
  beltgraph
  PUBLIC
  "include"
)

target_compile_definitions(
  beltgraph
  PUBLIC
  GRAPHER
)
