#include <beltgraph.h>
#include <stdio.h>
#include <time.h>

FILE *in = NULL;
void read_cb(bgOffset_t pos, void *buffer, bgSize_t size) {
  fseek(in, pos, SEEK_SET);
  fread(buffer, size, 1, in);
}

int main(void) {
  bgError_t err;
  
  /* プロローグ */
  in = fopen("bg/minato.bg", "rb");
  bgReader_t reader;
  bgReaderCreate(&reader, read_cb);

  bgHeader_t header;
  bgHeaderCreate(&header);
  bgHeaderFetch(&header, &reader);
  bgNodeContext_t nc;
  bgNodeContextCreate(&nc, &header);
  bgNodeContextFetch(&nc, &reader, &header);
  bgWayContext_t wc;
  bgWayContextCreate(&wc, &header);
  bgWayContextFetch(&wc, &reader, &header);

  bgNode_t node;
  bgNodeCreate(&node, &header);
  bgWay_t way;
  bgWayCreate(&way, &header);

  bgAStarVisited_t visited;
  bgAStarVisitedCreate(&visited, header.node_count, 0);
  bgAStarHeap_t heap;
  bgAStarHeapCreate(&heap, header.node_count);
  bgPath_t path;
  bgPathCreate(&path, header.node_count);

  bgNodeFetchNearest(&node, &reader, &header, 139.751, 35.635);
  bgIdx_t start_node_idx = node.idx;
  bgNodeFetchNearest(&node, &reader, &header, 139.800, 35.635);
  bgIdx_t goal_node_idx = node.idx;

  /* 経路探索 */
  for(bgIdx_t i = 0; i < wc.dim; i++) {
    bgWayContextSetWeight(&wc, i, 0);
  }
  for(bgIdx_t i = 0; i < nc.dim; i++) {
    bgNodeContextSetWeight(&nc, i, 0);
  }

  clock_t start = clock();
  err = bgAStar(
    &reader, &header,
    &nc, &wc,
    &node, &way,
    &visited, &heap,
    start_node_idx, goal_node_idx,
    &path
  );
  clock_t end = clock();

  if(err == BG_ERROR_OK) {
    printf("Ok !\n");

    printf("path = {");
    for(bgIdx_t i = 0; i < path.size; i++) {
      printf("%u -> ", path.node_idx_order[i]);
    }
    printf("\b\b\b\b}    \n\n");
  }else {
    printf("Err !\n");
  }
  printf("time: %.4f sec\n", (double)(end - start) / CLOCKS_PER_SEC);
  printf("|visited| allocated: %u, used: %u\n", visited.capacity, visited.size);
  printf("|heap   | allocated: %u, used: %u\n", heap.capacity, heap.peak_size);
  printf("|path   | allocated: %u, used: %u\n", path.capacity, path.size);

  /* エピローグ */
  bgPathDestroy(&path);
  bgAStarHeapDestroy(&heap);
  bgAStarVisitedDestroy(&visited);
  bgWayDestroy(&way);
  bgNodeDestroy(&node);
  bgWayContextDestroy(&wc);
  bgNodeContextDestroy(&nc);
  bgHeaderDestroy(&header);
  bgReaderDestroy(&reader);
  fclose(in);
  return 0;
}