#include "experiment.h"

#define STEP 100

/**
 * ウェイ重みを-1から1へ変化させて、生成される経路を記録します。
 * #define STEPの数だけ変化させます
 * 指定した重み以外はすべて0です。
 * 
 * コスト計算のようにウェイ指標値を長さスケールします。
 */

bgIndicator_t path_total_ind(bgIdx_t ind_idx) {
  bgIndicator_t sum = 0;
  
  for(bgIdx_t i = 1; i < e_path.size; i++) {
    bgNodeFetch(&e_node, &e_reader, &e_header, e_path.node_idx_order[i-1]);
    for(bgIdx_t j = e_node.way_begin; j < e_node.way_end; j++) {
      bgWayFetch(&e_way, &e_reader, &e_header, j);
      if(e_way.to != e_path.node_idx_order[i]) continue;

      sum += e_way.indicators[ind_idx] * e_way.length;
      
      break;
    }
  }

  return sum;
}

int main(void) {
  const char *bg_file = getenv("BG_FILE");

if(bg_file == NULL) {
  fprintf(stderr, "BG_FILE is not set\n");
  return 1;
}

e_init(bg_file, E_READ_RAM);

  FILE *out = fopen(e_output_path("waystepweight.plt"), "w");

  bgNodeFetchNearest(&e_node, &e_reader, &e_header, 139.751, 35.635);
  bgIdx_t start_node_idx = e_node.idx;
  bgNodeFetchNearest(&e_node, &e_reader, &e_header, 139.800, 35.635);
  bgIdx_t goal_node_idx = e_node.idx;

  e_weight_zero();
  bgIdx_t ind_idx = e_way_ind_select();

  for(int i = 0; i <= STEP; i++) {
    bgWeight_t w = (2.0 / STEP) * i - 1.0;
    
    bgWayContextSetWeight(&e_wc, ind_idx, w);
    e_astar(start_node_idx, goal_node_idx, 1);

    fprintf(out, "%f %f\n", w, path_total_ind(ind_idx));
  }

  fclose(out);
  e_end();
  return 0;
}

/**
 * gnuplot
 * plot "experiments/waystepweight.plt" u 1:2 w p
 */
