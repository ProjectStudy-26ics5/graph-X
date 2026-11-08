function(add_experiment e)
  add_executable(${e})
    target_sources(
    ${e}
    PRIVATE
    "${e}.c"
  )
  target_link_libraries(
    ${e}
    PRIVATE
    PS::beltgraph
  )
endfunction()

add_experiment(bgplot)
add_experiment(pathplot)
add_experiment(wayindplot)
add_experiment(nodeindplot)
add_experiment(pathtime)
add_experiment(nodestepweight)
add_experiment(waystepweight)
find_package(expat CONFIG REQUIRED)
find_package(BZip2 REQUIRED)
find_package(ZLIB REQUIRED)

add_executable(coverage coverage.cpp)
target_link_libraries(coverage PRIVATE
  PS::beltgraph
  expat::expat
  BZip2::BZip2
  ZLIB::ZLIB
)