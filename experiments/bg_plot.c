#include "experiment.h"

int main(void) {
  e_init("bg/minato.bg");
  FILE *out = fopen("experiments/bg_plot.plt", "w");

  for(bgIdx_t i = 0; i < e_header.node_count; i++) {
    bgNodeFetch(&e_node, &e_reader, &e_header, i);
    fprintf(out, "%lf %lf\n", e_node.lon, e_node.lat);
  }

  fclose(out);
  e_end();
  return 0;
}
