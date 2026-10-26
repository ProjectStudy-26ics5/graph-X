#include "experiment.h"

/**
 * gnuplotを用いる、ウェイの指標値を色でを書き出します
 */

int main(void) {
  e_init("bg/minato.bg", E_READ_RAM);
  FILE *out = fopen("experiments/wayindplot.plt", "w");

  bgIdx_t ind_idx = e_way_ind_select();

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
      fprintf(out, "%lf %lf %lf %f %lf\n", lon, lat, dlon, dlat, e_way.indicators[ind_idx]);
    }
  }

  fclose(out);
  e_end();
  return 0;
}

/**
 * gnuplot
 * set cbrange [0.0:1.0]
 * set palette viridis
 * plot "experiments/wayindplot.plt" u 1:2:3:4:5 w vec nohead lc palette lw 1
 */
