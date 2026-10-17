#include <beltgraph.h>
#include <stdio.h>

FILE *in = NULL;
void read_cb(bgOffset_t pos, void *buffer, bgSize_t size) {
  fseek(in, pos, SEEK_SET);
  fread(buffer, size, 1, in);
}

int main(void) {
  in = fopen("bg/minato.bg", "rb");
  bgReader_t reader;
  bgReaderCreate(&reader, read_cb);

  bgHeader_t header;
  bgHeaderCreate(&header);
  bgHeaderFetch(&header, &reader);

  bgNode_t node;
  bgNodeCreate(&node, &header);

  bgNodeFetchNearest(&node, &reader, &header, 139.751, 35.635);
  printf("nearest lon = %lf\n", node.lon);
  printf("nearest lat = %lf\n", node.lat);
  
  bgNodeDestroy(&node);
  
  bgHeaderDestroy(&header);

  bgReaderDestroy(&reader);
  fclose(in);
  return 0;
}
