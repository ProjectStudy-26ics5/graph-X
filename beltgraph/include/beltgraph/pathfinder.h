#ifndef BELTGRAPH_PATHFINDER_H
#define BELTGRAPH_PATHFINDER_H

#include <beltgraph/types.h>

#ifdef __cplusplus
extern "C" {
#endif

struct bgVisited {
  // SoAの構造です。
  
  // hash値はノードのidxです
  bgIdx_t *hash;

  // chainは
  // bgVisited::sizeであるときは、chain終了
  // 0であるときは、このバケットが未割当
  bgIdx_t *chain;

  // Nodeが持っているWayのうち、前回のノードにつながるもの
  // 出次数が255を超えることはめったにないため
  // 8bit(1byte)のデータに圧縮できます
  uint8_t *node_transition;

  bgIdx_t capacity;
  bgIdx_t size;
  bgIdx_t table_size;
  bgIdx_t chain_top;
};

bgError_t bgVisitedCreate(bgVisited_t *visited, bgIdx_t table_size, bgIdx_t chain_size);
void bgVisitedDestroy(bgVisited_t *visited);
int bgVisitedGet(const bgVisited_t *visited, bgIdx_t hash, uint8_t *node_transition);
bgError_t bgVisitedInsert(bgVisited_t *visited, bgIdx_t hash, uint8_t node_transition);



struct bgFrontier {
  // SoAの構造です。

  // 指標値は厳密な数値ではないので、
  // weightとcostは半精度浮動小数点を使えるかもしれません
  bgCost_t *weight;
  bgCost_t *cost;
  bgIdx_t  *node_idx;

  bgIdx_t capacity;
  bgIdx_t size;
};

bgError_t bgFrontierCreate(bgFrontier_t *frontier, bgIdx_t capacity);
void bgFrontierDestroy(bgFrontier_t *frontier);
bgError_t bgFrontierPop(bgFrontier_t *frontier);
bgError_t bgFrontierPush(bgFrontier_t *frontier, bgCost_t weight, bgCost_t cost, bgIdx_t node_idx);
bgError_t bgFrontierTop(const bgFrontier_t *frontier, bgCost_t *weight, bgCost_t *cost, bgIdx_t *node_idx);



struct bgPath {
  bgIdx_t *node_idx_order;
  bgIdx_t capacity;
  bgIdx_t size;
};

bgError_t bgPathCreate(bgPath_t *path, bgIdx_t capacity);
void bgPathDestroy(bgPath_t *path);

#ifdef __cplusplus
}
#endif

#endif
