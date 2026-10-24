#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <beltgraph.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * これは実験をスムーズに行うための、ヘッダオンリーのヘルパーです。
 * grapher、beltgraphライブラリを代表するものではありません。
 */



static FILE *e_file = NULL;
static char *e_file_buffer = NULL;
static bgReader_t e_reader;
static bgHeader_t e_header;
static bgNodeContext_t e_nc;
static bgWayContext_t e_wc;
static bgNode_t e_node;
static bgWay_t e_way;
static bgAStarVisited_t e_visited;
static bgAStarHeap_t e_heap;
static bgPath_t e_path;

enum e_read_type {
  E_READ_RAM, // bgファイルデータをキャッシュしてから使います
  E_READ_ROM, // bgファイル直読み
};

static void e_ram_read_cb(bgOffset_t pos, void *buffer, bgSize_t size) {
  memcpy(buffer, e_file_buffer + pos, size);
}

static void e_rom_read_cb(bgOffset_t pos, void *buffer, bgSize_t size) {
  fseek(e_file, pos, SEEK_SET);
  fread(buffer, size, 1, e_file);
}

static void e_init(const char *file_path, enum e_read_type read_type) {
  e_file = fopen(file_path, "rb");
  if(read_type == E_READ_RAM) {
    fseek(e_file, 0, SEEK_END);
    long long e_file_size = ftell(e_file);
    fseek(e_file, 0, SEEK_SET);

    e_file_buffer = malloc(e_file_size);
    fread(e_file_buffer, 1, e_file_size, e_file);
  }
  
  if(read_type == E_READ_RAM) {
    bgReaderCreate(&e_reader, e_ram_read_cb);
  }else {
    bgReaderCreate(&e_reader, e_rom_read_cb);
  }

  bgHeaderCreate(&e_header);
  bgHeaderFetch(&e_header, &e_reader);

  bgNodeContextCreate(&e_nc, &e_header);
  bgNodeContextFetch(&e_nc, &e_reader, &e_header);
  
  bgWayContextCreate(&e_wc, &e_header);
  bgWayContextFetch(&e_wc, &e_reader, &e_header);
  
  bgNodeCreate(&e_node, &e_header);

  bgWayCreate(&e_way, &e_header);

  bgAStarVisitedCreate(&e_visited, e_header.node_count, 0);
  bgAStarHeapCreate(&e_heap, e_header.node_count);
  bgPathCreate(&e_path, e_header.node_count);
}

static void e_end() {
  fclose(e_file);
  e_file = NULL;
  free(e_file_buffer);
  e_file_buffer = NULL;
  
  bgReaderDestroy(&e_reader);

  bgHeaderDestroy(&e_header);

  bgNodeContextDestroy(&e_nc);

  bgWayContextDestroy(&e_wc);

  bgNodeDestroy(&e_node);

  bgWayDestroy(&e_way);

  bgAStarVisitedDestroy(&e_visited);
  bgAStarHeapDestroy(&e_heap);
  bgPathDestroy(&e_path);
}

static void e_weight_zero() {
  for(bgIdx_t i = 0; i < e_wc.dim; i++) {
    bgWayContextSetWeight(&e_wc, i, 0);
  }
  for(bgIdx_t i = 0; i < e_nc.dim; i++) {
    bgNodeContextSetWeight(&e_nc, i, 0);
  }
}

static void e_weight_input() {
  bgWeight_t w;
  for(bgIdx_t i = 0; i < e_wc.dim; i++) {
    printf("%s: %s\n", bgWayContextIndicatorName(&e_wc, i), bgWayContextIndicatorDesc(&e_wc, i));
    printf("weight = ");
    scanf("%f", &w);
    bgWayContextSetWeight(&e_wc, i, w);
  }
  for(bgIdx_t i = 0; i < e_nc.dim; i++) {
    printf("%s: %s\n", bgNodeContextIndicatorName(&e_nc, i), bgNodeContextIndicatorDesc(&e_nc, i));
    printf("weight = ");
    scanf("%f", &w);
    bgNodeContextSetWeight(&e_nc, i, w);
  }
}

#endif
