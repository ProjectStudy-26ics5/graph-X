#include <beltgraph.h>
#include <stdlib.h>
#include <string.h>

#define BUCKET_END_SIGN(visited) ((visited)->size)
#define BUCKET_UNALLOCATED_SIGN 0
#define UNALLOCATED_BUCKET(visited, i) ((visited)->chain[i] == BUCKET_UNALLOCATED_SIGN)

bgError_t bgVisitedCreate(bgVisited_t *visited, bgIdx_t table_size, bgIdx_t chain_size) {
  const bgIdx_t size = table_size + chain_size;

  visited->hash = malloc(size * sizeof(bgIdx_t));
  visited->chain = malloc(size * sizeof(bgIdx_t));
  visited->node_transition = malloc(size * sizeof(uint8_t));
  visited->size = size;
  visited->table_size = table_size;
  visited->chain_top = table_size;

  if(visited->hash == NULL) {
    return BG_ERROR_ALLOC;
  }
  if(visited->chain == NULL) {
    return BG_ERROR_ALLOC;
  }
  if(visited->node_transition == NULL) {
    return BG_ERROR_ALLOC;
  }

  for(bgIdx_t i = 0; i < size; i++) {
    visited->chain[i] = BUCKET_UNALLOCATED_SIGN;
  }

  return BG_ERROR_OK;
}

void bgVisitedDestroy(bgVisited_t *visited) {
  free(visited->hash);
  free(visited->chain);
  free(visited->node_transition);
}

int bgVisitedGet(const bgVisited_t *visited, bgIdx_t hash, uint8_t *node_transition) {
  for(bgIdx_t i = hash % visited->table_size; i != BUCKET_END_SIGN(visited); i = visited->chain[i]) {
    if(UNALLOCATED_BUCKET(visited, i)) break;

    if(visited->hash[i] == hash) {
      if(node_transition != NULL) {
        *node_transition = visited->node_transition[i];
      }
      return 1;
    }
  }

  return 0;
}

bgError_t bgVisitedInsert(bgVisited_t *visited, bgIdx_t hash, uint8_t node_transition) {
  // 要素がすでに存在しても追加します
  bgIdx_t i = hash % visited->table_size;

  if(UNALLOCATED_BUCKET(visited, i)) {
    visited->hash[i] = hash;
    visited->chain[i] = BUCKET_END_SIGN(visited);
    visited->node_transition[i] = node_transition;

    return BG_ERROR_OK;
  }

  if(visited->chain_top == visited->size) return BG_ERROR_MEM;
  bgIdx_t j = visited->chain_top++;

  visited->hash[j] = hash;
  visited->chain[j] = visited->chain[i];
  visited->node_transition[j] = node_transition;

  visited->chain[i] = j;

  return BG_ERROR_OK;
}
