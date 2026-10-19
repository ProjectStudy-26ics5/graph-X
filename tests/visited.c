#include <beltgraph.h>
#include <stdio.h>

void print_visited(const bgVisited_t *visited, bgIdx_t node_count) {
  printf("contain {");
  for(bgIdx_t i = 0; i < node_count; i++) {
    if(bgVisitedGet(visited, i, NULL)) {
      printf("%u, ", i);
    }
  }
  printf("\b\b} (size = %u)\n", visited->size);
}

int main(void) {
  bgVisited_t visited;
  bgVisitedCreate(&visited, 2, 2);

  const bgIdx_t node_count = 5;
  printf("bucket count = %u\n", visited.capacity);
  printf("node count = %u\n", node_count);

  for(bgIdx_t i = 0; i < node_count; i++) {
    bgError_t err = bgVisitedInsert(&visited, i, 0);
    if(err != BG_ERROR_OK) {
      printf("visited insert failed !\n");
      break;
    }

    print_visited(&visited, node_count);
  }

  bgVisitedDestroy(&visited);
  return 0;
}
