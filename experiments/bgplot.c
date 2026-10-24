#include "experiment.h"

/**
 * gnuplotを用いる、bgデータのグラフを書き出します
 */

int main(void) {
  e_init("bg/minato.bg");
  FILE *out = fopen("experiments/bgplot.plt", "w");

  for(bgIdx_t i = 0; i < e_header.node_count; i++) {
    bgNodeFetch(&e_node, &e_reader, &e_header, i);
    double lon = e_node.lon;
    double lat = e_node.lat;
    bgIdx_t way_begin = e_node.way_begin;
    bgIdx_t way_end = e_node.way_end;

    for(bgIdx_t j = way_begin; j < way_end; j++) {
      bgWayFetch(&e_way, &e_reader, &e_header, j);
      bgNodeFetch(&e_node, &e_reader, &e_header, e_way.to);

      double dlon = e_node.lon - lon;
      double dlat = e_node.lat - lat;
      fprintf(out, "%lf %lf %lf %f\n", lon, lat, dlon, dlat);
    }
  }

  fclose(out);
  e_end();
  return 0;
}

/**
 * gnuplot
 * plot "experiments/bgplot.plt" u 1:2:3:4 w vec nohead lc rgb "black" lw 1
 */
