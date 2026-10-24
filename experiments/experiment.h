#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <beltgraph.h>
#include <stdio.h>

/**
 * これは実験をスムーズに行うための、ヘッダオンリーのヘルパーです。
 * grapher、beltgraphライブラリを代表するものではありません。
 */



static FILE *e_file = NULL;
static bgReader_t e_reader;
static bgHeader_t e_header;
static bgNodeContext_t e_nc;
static bgWayContext_t e_wc;
static bgNode_t e_node;
static bgWay_t e_way;

static void e_rom_read_cb(bgOffset_t pos, void *buffer, bgSize_t size) {
  fseek(e_file, pos, SEEK_SET);
  fread(buffer, size, 1, e_file);
}

static void e_init(const char *file_path) {
  e_file = fopen(file_path, "rb");
  bgReaderCreate(&e_reader, e_rom_read_cb);

  bgHeaderCreate(&e_header);
  bgHeaderFetch(&e_header, &e_reader);

  bgNodeContextCreate(&e_nc, &e_header);
  bgNodeContextFetch(&e_nc, &e_reader, &e_header);
  
  bgWayContextCreate(&e_wc, &e_header);
  bgWayContextFetch(&e_wc, &e_reader, &e_header);
  
  bgNodeCreate(&e_node, &e_header);

  bgWayCreate(&e_way, &e_header);
}

static void e_end() {
  fclose(e_file);
  bgReaderDestroy(&e_reader);

  bgHeaderDestroy(&e_header);

  bgNodeContextDestroy(&e_nc);

  bgWayContextDestroy(&e_wc);

  bgNodeDestroy(&e_node);

  bgWayDestroy(&e_way);
}

#endif
