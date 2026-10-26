#include "experiment.h"

/**
 * gnuplotを用いる、経路データを書き出します
 */

int main(void) {
  e_init("bg/minato.bg", E_READ_RAM);
  FILE *out = fopen("experiments/pathplot.plt", "w");
  
  e_weight_input();

  bgNodeFetchNearest(&e_node, &e_reader, &e_header, 139.751, 35.635);
  bgIdx_t start_node_idx = e_node.idx;
  bgNodeFetchNearest(&e_node, &e_reader, &e_header, 139.800, 35.635);
  bgIdx_t goal_node_idx = e_node.idx;

  e_astar(start_node_idx, goal_node_idx, 1);

  for(bgIdx_t i = 1; i < e_path.size; i++) {
    bgNodeFetch(&e_node, &e_reader, &e_header, e_path.node_idx_order[i-1]);
    double lon = e_node.lon;
    double lat = e_node.lat;

    bgNodeFetch(&e_node, &e_reader, &e_header, e_path.node_idx_order[i]);
    double dlon = e_node.lon - lon;
    double dlat = e_node.lat - lat;

    fprintf(out, "%lf %lf %lf %lf\n", lon, lat, dlon, dlat);
  }

  fclose(out);
  e_end();
  return 0;
}

/**
 * gnuplot
 * plot "experiments/bgplot.plt" u 1:2:3:4 w vec nohead lc rgb "black" lw 1, "experiments/pathplot.plt" u 1:2:3:4 w vec nohead lc rgb "red" lw 3
 */
