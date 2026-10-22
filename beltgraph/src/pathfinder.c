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

void bgAStarVisitedClear(bgAStarVisited_t *visited) {
  visited->size = 0;
  visited->chain_top = visited->table_size;

  for(bgIdx_t i = 0; i < visited->capacity; i++) {
    visited->chain[i] = BUCKET_UNALLOCATED_SIGN;
  }
}

int bgAStarVisitedFind(const bgAStarVisited_t *visited, bgIdx_t hash, uint8_t *node_transition) {
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



bgError_t bgAStarHeapCreate(bgAStarHeap_t *heap, bgIdx_t capacity) {
  heap->weight = malloc(capacity * sizeof(bgCost_t));
  heap->cost = malloc(capacity * sizeof(bgCost_t));
  heap->node_idx = malloc(capacity * sizeof(bgIdx_t));
  heap->node_transition = malloc(capacity * sizeof(uint8_t));
  heap->capacity = capacity;
  heap->size = 0;

  if(heap->weight == NULL) {
    return BG_ERROR_ALLOC;
  }
  if(heap->cost == NULL) {
    return BG_ERROR_ALLOC;
  }
  if(heap->node_idx == NULL) {
    return BG_ERROR_ALLOC;
  }

  return BG_ERROR_OK;
}

void bgAStarHeapDestroy(bgAStarHeap_t *heap) {
  free(heap->weight);
  free(heap->cost);
  free(heap->node_idx);
  free(heap->node_transition);

  heap->weight = NULL;
  heap->cost = NULL;
  heap->node_idx = NULL;
  heap->node_transition = NULL;
  heap->capacity = 0;
  heap->size = 0;
}

void bgAStarHeapClear(bgAStarHeap_t *heap) {
  heap->size = 0;
}

bgError_t bgAStarHeapPop(bgAStarHeap_t *heap) {
  if(heap->size == 0) {
    return BG_ERROR_LOGIC;
  }

  const bgIdx_t back = --heap->size;
  bgIdx_t p = 0;
  bgIdx_t c;

  while((c = (p << 1) + 1) < heap->size) {
    if(c + 1 < heap->size && heap->weight[c] > heap->weight[c + 1]) {
      c = c + 1;
    }
    if(heap->weight[back] <= heap->weight[c]) break;

    heap->weight[p] = heap->weight[c];
    heap->cost[p] = heap->cost[c];
    heap->node_idx[p] = heap->node_idx[c];
    heap->node_transition[p] = heap->node_transition[c];

    p = c;
  }

  heap->weight[p] = heap->weight[back];
  heap->cost[p] = heap->cost[back];
  heap->node_idx[p] = heap->node_idx[back];
  heap->node_transition[p] = heap->node_transition[back];

  return BG_ERROR_OK;
}

bgError_t bgAStarHeapPush(
  bgAStarHeap_t *heap,
  bgCost_t weight,
  bgCost_t cost, bgIdx_t node_idx, uint8_t node_transition
) {
  if(heap->size == heap->capacity) {
    return BG_ERROR_CAP;
  }

  const bgIdx_t back = heap->size++;
  bgIdx_t p;
  bgIdx_t c = back;

  while(c != 0) {
    p = (c - 1) >> 1;
    if(heap->weight[p] <= weight) break;

    heap->weight[c] = heap->weight[p];
    heap->cost[c] = heap->cost[p];
    heap->node_idx[c] = heap->node_idx[p];
    heap->node_transition[c] = heap->node_transition[p];
    
    c = p;
  }

  heap->weight[c] = weight;
  heap->cost[c] = cost;
  heap->node_idx[c] = node_idx;
  heap->node_transition[c] = node_transition;

  return BG_ERROR_OK;
}

bgError_t bgAStarHeapTop(
  const bgAStarHeap_t *heap,
  bgCost_t *weight,
  bgCost_t *cost, bgIdx_t *node_idx, uint8_t *node_transition
) {
  if(heap->size == 0) {
    return BG_ERROR_LOGIC;
  }

  if(weight != NULL) {
    *weight = heap->weight[0];
  }
  if(cost != NULL) {
    *cost = heap->cost[0];
  }
  if(node_idx != NULL) {
    *node_idx = heap->node_idx[0];
  }
  if(node_transition != NULL) {
    *node_transition = heap->node_transition[0];
  }

  return BG_ERROR_OK;
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

void bgPathClear(bgPath_t *path) {
  path->size = 0;
}
