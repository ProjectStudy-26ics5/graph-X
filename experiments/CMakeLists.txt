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
