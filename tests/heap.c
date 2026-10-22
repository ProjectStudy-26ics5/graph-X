#include <beltgraph.h>
#include <stdio.h>

int main(void) {
  bgError_t err;
  bgAStarHeap_t heap;
  bgAStarHeapCreate(&heap, 3);

  bgAStarHeapPush(&heap, 10, 0, 0, 0);
  bgAStarHeapPush(&heap, 1, 0, 0, 0);
  bgAStarHeapPush(&heap, 5, 0, 0, 0);

  while(heap.size) {
    bgCost_t heap_weight;
    bgAStarHeapTop(&heap, &heap_weight, NULL, NULL, NULL);
    bgAStarHeapPop(&heap);

    printf("min heap weight = %f\n", heap_weight);
  }

  err = bgAStarHeapPop(&heap);
  if(err != BG_ERROR_OK) {
    printf("heap is empty !\n");
  }

  for(int i = 0; i < 100; i++) {
    err = bgAStarHeapPush(&heap, 0, 0, 0, 0);
    if(err != BG_ERROR_OK) {
      printf("heap is full ! (i = %d)\n", i);
      break;
    }
  }
  
  bgAStarHeapDestroy(&heap);
}