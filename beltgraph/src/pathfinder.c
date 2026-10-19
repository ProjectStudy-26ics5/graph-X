#include <beltgraph.h>
#include <stdlib.h>
#include <string.h>

#define BUCKET_END_SIGN(visited) ((visited)->capacity)
#define BUCKET_UNALLOCATED_SIGN 0
#define UNALLOCATED_BUCKET(visited, i) ((visited)->chain[i] == BUCKET_UNALLOCATED_SIGN)

bgError_t bgAStarVisitedCreate(bgAStarVisited_t *visited, bgIdx_t table_size, bgIdx_t chain_size) {
  const bgIdx_t capacity = table_size + chain_size;

  visited->hash = malloc(capacity * sizeof(bgIdx_t));
  visited->chain = malloc(capacity * sizeof(bgIdx_t));
  visited->node_transition = malloc(capacity * sizeof(uint8_t));
  visited->capacity = capacity;
  visited->size = 0;
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

  for(bgIdx_t i = 0; i < capacity; i++) {
    visited->chain[i] = BUCKET_UNALLOCATED_SIGN;
  }

  return BG_ERROR_OK;
}

void bgAStarVisitedDestroy(bgAStarVisited_t *visited) {
  free(visited->hash);
  free(visited->chain);
  free(visited->node_transition);

  visited->hash = NULL;
  visited->chain = NULL;
  visited->node_transition = NULL;
  visited->capacity = 0;
  visited->size = 0;
  visited->table_size = 0;
  visited->chain_top = 0;
}

int bgAStarVisitedGet(const bgAStarVisited_t *visited, bgIdx_t hash, uint8_t *node_transition) {
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

bgError_t bgAStarVisitedInsert(bgAStarVisited_t *visited, bgIdx_t hash, uint8_t node_transition) {
  // 要素がすでに存在しても追加します
  bgIdx_t i = hash % visited->table_size;

  if(UNALLOCATED_BUCKET(visited, i)) {
    visited->hash[i] = hash;
    visited->chain[i] = BUCKET_END_SIGN(visited);
    visited->node_transition[i] = node_transition;

    visited->size++;
    return BG_ERROR_OK;
  }

  if(visited->chain_top == visited->capacity) return BG_ERROR_CAP;
  bgIdx_t j = visited->chain_top++;

  visited->hash[j] = hash;
  visited->chain[j] = visited->chain[i];
  visited->node_transition[j] = node_transition;

  visited->chain[i] = j;

  visited->size++;
  return BG_ERROR_OK;
}



bgError_t bgAStarHeapCreate(bgAStarHeap_t *frontier, bgIdx_t capacity) {
  frontier->weight = malloc(capacity * sizeof(bgCost_t));
  frontier->cost = malloc(capacity * sizeof(bgCost_t));
  frontier->node_idx = malloc(capacity * sizeof(bgIdx_t));
  frontier->node_transition = malloc(capacity * sizeof(uint8_t));
  frontier->capacity = capacity;
  frontier->size = 0;

  if(frontier->weight == NULL) {
    return BG_ERROR_ALLOC;
  }
  if(frontier->cost == NULL) {
    return BG_ERROR_ALLOC;
  }
  if(frontier->node_idx == NULL) {
    return BG_ERROR_ALLOC;
  }

  return BG_ERROR_OK;
}

void bgAStarHeapDestroy(bgAStarHeap_t *frontier) {
  free(frontier->weight);
  free(frontier->cost);
  free(frontier->node_idx);
  free(frontier->node_transition);

  frontier->weight = NULL;
  frontier->cost = NULL;
  frontier->node_idx = NULL;
  frontier->node_transition = NULL;
  frontier->capacity = 0;
  frontier->size = 0;
}



bgError_t bgPathCreate(bgPath_t *path, bgIdx_t capacity) {
  path->node_idx_order = malloc(capacity * sizeof(bgIdx_t));
  path->capacity = capacity;
  path->size = 0;

  if(path->node_idx_order == NULL) {
    return BG_ERROR_ALLOC;
  }

  return BG_ERROR_OK;
}

void bgPathDestroy(bgPath_t *path) {
  free(path->node_idx_order);

  path->node_idx_order = NULL;
  path->capacity = 0;
  path->size = 0;
}
