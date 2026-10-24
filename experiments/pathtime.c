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
  bgError_t err;
  
  // RAM
  e_init("bg/minato.bg", E_READ_RAM);

  for(bgIdx_t i = 0; i < e_wc.dim; i++) {
    bgWayContextSetWeight(&e_wc, i, 0);
  }
  for(bgIdx_t i = 0; i < e_nc.dim; i++) {
    bgNodeContextSetWeight(&e_nc, i, 0);
  }

  bgNodeFetchNearest(&e_node, &e_reader, &e_header, 139.751, 35.635);
  start_node_idx = e_node.idx;
  bgNodeFetchNearest(&e_node, &e_reader, &e_header, 139.800, 35.635);
  goal_node_idx = e_node.idx;

  start = clock();
  err = bgAStar(
    &e_reader, &e_header,
    &e_nc, &e_wc,
    &e_node, &e_way,
    &e_visited, &e_heap,
    start_node_idx, goal_node_idx,
    1,
    &e_path
  );
  end = clock();
  if(err == BG_ERROR_OK) {
    printf("RAM A* time: %.4lf sec\n", (double)(end - start) / CLOCKS_PER_SEC);
  }

  e_end();

  // ROM
  e_init("bg/minato.bg", E_READ_ROM);

  for(bgIdx_t i = 0; i < e_wc.dim; i++) {
    bgWayContextSetWeight(&e_wc, i, 0);
  }
  for(bgIdx_t i = 0; i < e_nc.dim; i++) {
    bgNodeContextSetWeight(&e_nc, i, 0);
  }

  bgNodeFetchNearest(&e_node, &e_reader, &e_header, 139.751, 35.635);
  start_node_idx = e_node.idx;
  bgNodeFetchNearest(&e_node, &e_reader, &e_header, 139.800, 35.635);
  goal_node_idx = e_node.idx;

  start = clock();
  err = bgAStar(
    &e_reader, &e_header,
    &e_nc, &e_wc,
    &e_node, &e_way,
    &e_visited, &e_heap,
    start_node_idx, goal_node_idx,
    1,
    &e_path
  );
  end = clock();
  if(err == BG_ERROR_OK) {
    printf("ROM A* time: %.4lf sec\n", (double)(end - start) / CLOCKS_PER_SEC);
  }

  e_end();
  return 0;
}
