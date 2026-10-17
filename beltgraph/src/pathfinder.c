#include <beltgraph.h>
#include <stdlib.h>

#define END_SIGN(visited) ((visited)->size)
#define END_BUCKET(bucket, visited) ((bucket).chain == END_SIGN(visited))
#define UNALLOCATED_SIGN 0
#define UNALLOCATED_BUCKET(bucket) ((bucket).chain == UNALLOCATED_SIGN)

bgError_t bgVisitedCreate(bgVisited_t *visited, bgIdx_t table_size, bgIdx_t chain_size) {
  const bgIdx_t size = table_size + chain_size;

  visited->buckets = malloc(size * sizeof(bgVisitedBucket_t));
  visited->size = size;
  visited->table_size = table_size;
  visited->chain_top = table_size;

  if(visited->buckets == NULL) {
    return BG_ERROR_ALLOC;
  }

  for(bgIdx_t i = 0; i < size; i++) {
    visited->buckets[i].chain = UNALLOCATED_SIGN;
  }
  
  return BG_ERROR_OK;
}

void bgVisitedDestroy(bgVisited_t *visited) {
  free(visited->buckets);
}

int bgVisitedGet(const bgVisited_t *visited, bgIdx_t hash, uint8_t *node_transition) {
  for(bgIdx_t i = hash % visited->table_size; i != END_SIGN(visited); i = visited->buckets[i].chain) {
    if(UNALLOCATED_BUCKET(visited->buckets[i])) break;

    if(visited->buckets[i].hash == hash) {
      if(node_transition != NULL) {
        *node_transition = visited->buckets[i].node_transition;
      }
      return 1;
    }
  }

  return 0;
}

bgError_t bgVisitedInsert(bgVisited_t *visited, bgIdx_t hash, uint8_t node_transition) {
  // 要素がすでに存在しても追加します
  bgIdx_t i = hash % visited->table_size;

  if(UNALLOCATED_BUCKET(visited->buckets[i])) {
    visited->buckets[i].hash = hash;
    visited->buckets[i].chain = END_SIGN(visited);
    visited->buckets[i].node_transition = node_transition;

    return BG_ERROR_OK;
  }

  if(visited->chain_top == visited->size) return BG_ERROR_MEM;
  bgIdx_t j = visited->chain_top++;

  visited->buckets[j].hash = hash;
  visited->buckets[j].chain = visited->buckets[i].chain;
  visited->buckets[j].node_transition = node_transition;

  visited->buckets[i].chain = j;

  return BG_ERROR_OK;
}
