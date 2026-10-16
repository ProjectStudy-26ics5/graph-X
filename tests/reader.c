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

  // header
  bgHeader_t header;
  bgHeaderCreate(&header);
  bgHeaderFetch(&header, &reader);

  printf("nc_pos = %llu\n", header.nc_pos);
  printf("nc_dim = %u\n", header.nc_dim);
  printf("nc_ind_name_size = %llu\n", header.nc_ind_name_size);
  printf("nc_ind_desc_size = %llu\n", header.nc_ind_desc_size);

  printf("wc_pos = %llu\n", header.wc_pos);
  printf("wc_dim = %u\n", header.wc_dim);
  printf("wc_ind_name_size = %llu\n", header.wc_ind_name_size);
  printf("wc_ind_desc_size = %llu\n", header.wc_ind_desc_size);
  
  printf("node_pos = %llu\n", header.node_pos);
  printf("node_count = %u\n", header.node_count);
  printf("node_stride = %llu\n", header.node_stride);

  printf("way_pos = %llu\n", header.way_pos);
  printf("way_count = %u\n", header.way_count);
  printf("way_stride = %llu\n", header.way_stride);

  bgHeaderDestroy(&header);

  // nc
  bgNodeContext_t nc;
  bgNodeContextCreate(&nc, &header);
  bgNodeContextFetch(&nc, &reader, &header);
  for(int i = 0; i < nc.dim; i++) {
    printf("%s: %s\n", bgNodeContextIndicatorName(&nc, i), bgNodeContextIndicatorDesc(&nc, i));
  }
  bgNodeContextDestroy(&nc);

  // wc
  bgWayContext_t wc;
  bgWayContextCreate(&wc, &header);
  bgWayContextFetch(&wc, &reader, &header);
  for(int i = 0; i < wc.dim; i++) {
    printf("%s: %s\n", bgWayContextIndicatorName(&wc, i), bgWayContextIndicatorDesc(&wc, i));
  }
  bgWayContextDestroy(&wc);

  bgReaderDestroy(&reader);
  fclose(in);
  return 0;
}
