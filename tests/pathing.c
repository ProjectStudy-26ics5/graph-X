#include <beltgraph.h>
#include <stdio.h>

FILE *in = NULL;
void read_cb(bgOffset_t pos, void *buffer, bgSize_t size) {
  fseek(in, pos, SEEK_SET);
  fread(buffer, size, 1, in);
}

int main(void) {
  bgError_t err;
  
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

  err = bgAStar(
    &reader, &header,
    &nc, &wc,
    &node, &way,
    &visited, &heap,
    start_node_idx, goal_node_idx,
    &path
  );
  if(err == BG_ERROR_OK) {
    printf("Ok !\n");
  }

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