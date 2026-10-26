#include "experiment.h"
#include <time.h>

/**
 * 経路探索の実行時間を調べます。
 * PCで実行するのでマイコン側の処理速度を代表するものではありません。
 * 
 * bgデータをメモリにコピーしてから探索を行うRAMと、
 * ファイル直読みでファイルシステムをたどるROMがあります。
 * 
 * やはり、bgをメモリにコピーしたほうが圧倒的に速いですね。
 */

int main(void) {
  clock_t start, end;
  bgIdx_t start_node_idx, goal_node_idx;
  bgCost_t heuristic_weight = 1;
  
  e_init("bg/minato.bg", E_READ_RAM);

  bgNodeFetchNearest(&e_node, &e_reader, &e_header, 139.751, 35.635);
  start_node_idx = e_node.idx;
  bgNodeFetchNearest(&e_node, &e_reader, &e_header, 139.800, 35.635);
  goal_node_idx = e_node.idx;

  // RAM
  e_weight_zero();
  start = clock();
  e_astar(start_node_idx, goal_node_idx, heuristic_weight);
  end = clock();
  printf("RAM A* time: %.4lf sec\n", (double)(end - start) / CLOCKS_PER_SEC);

  e_end();

  // ROM
  e_init("bg/minato.bg", E_READ_ROM);

  e_weight_zero();
  start = clock();
  e_astar(start_node_idx, goal_node_idx, heuristic_weight);
  end = clock();
  printf("ROM A* time: %.4lf sec\n", (double)(end - start) / CLOCKS_PER_SEC);

  e_end();
  return 0;
}
