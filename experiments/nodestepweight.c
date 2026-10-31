#include "experiment.h"

#define STEP 100

/**
 * ノード重みを-1から1へ変化させて、生成される経路を記録します。
 * #define STEPの数だけ変化させます
 * 指定した重み以外はすべて0です。
 * 
 * コスト計算のようにμをかけます。
 */

bgIndicator_t path_total_ind(bgIdx_t ind_idx) {
  bgIndicator_t sum = 0;
  
  for(bgIdx_t i = 0; i < e_path.size; i++) {
    bgNodeFetch(&e_node, &e_reader, &e_header, e_path.node_idx_order[i]);
    sum += e_node.indicators[ind_idx] * 1.0 / 10000.0;
  }

  return sum;
}

int main(void) {
  e_init("bg/minato.bg", E_READ_RAM);
  FILE *out = fopen(e_output_path("nodestepweight.plt"), "w");

  bgNodeFetchNearest(&e_node, &e_reader, &e_header, 139.751, 35.635);
  bgIdx_t start_node_idx = e_node.idx;
  bgNodeFetchNearest(&e_node, &e_reader, &e_header, 139.800, 35.635);
  bgIdx_t goal_node_idx = e_node.idx;

  e_weight_zero();
  bgIdx_t ind_idx = e_node_ind_select();

  for(int i = 0; i <= STEP; i++) {
    bgWeight_t w = (2.0 / STEP) * i - 1.0;
    
    bgNodeContextSetWeight(&e_nc, ind_idx, w);
    e_astar(start_node_idx, goal_node_idx, 1);

    fprintf(out, "%f %f\n", w, path_total_ind(ind_idx));
  }

  fclose(out);
  e_end();
  return 0;
}

/**
 * gnuplot
 * plot "experiments/nodestepweight.plt" u 1:2 w p
 */
