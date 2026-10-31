#include "experiment.h"

/**
 * gnuplotを用いる、ノードの指標値を色でを書き出します
 */

int main(void) {
  e_init("bg/minato.bg", E_READ_RAM);
  FILE *out = fopen(e_output_path("nodeindplot.plt"), "w");

  bgIdx_t ind_idx = e_node_ind_select();

  for(bgIdx_t i = 0; i < e_header.node_count; i++) {
    bgNodeFetch(&e_node, &e_reader, &e_header, i);
    double lon = e_node.lon;
    double lat = e_node.lat;
    
    fprintf(out, "%lf %lf %lf\n", lon, lat, e_node.indicators[ind_idx]);
  }

  fclose(out);
  e_end();
  return 0;
}

/**
 * gnuplot
 * set cbrange [0.0:1.0]
 * set palette viridis
 * plot "experiments/bgplot.plt" u 1:2:3:4 w vec nohead lc rgb "black" lw 1, "experiments/nodeindplot.plt" u 1:2:3 w p pt 7 ps 0.5 lc palette
 */
