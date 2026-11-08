#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <beltgraph.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * これは実験をスムーズに行うための、ヘッダオンリーのヘルパーです。
 * grapher、beltgraphライブラリを代表するものではありません。
 * 
 * "e_"から始まる関数やグローバル変数を用意します。
 */

static const char *e_env(const char *name, const char *fallback) {
  const char *value = getenv(name);
  return value ? value : fallback;
}

static double e_lon(const char *name, double fallback) {
  const char *value = getenv(name);
  return value ? atof(value) : fallback;
}

static char e_output[512];

static const char *e_output_path(const char *name) {
  snprintf(e_output, sizeof(e_output), "%s/%s",
    e_env("RESULT_DIR", "experiments"), e_env("RESULT_FILE", name));
  return e_output;
}

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

/**
 * 実験初期化
 */
static void e_init(const char *file_path, enum e_read_type read_type) {
  file_path = e_env("BG_FILE", file_path);
  e_file = fopen(file_path, "rb");

  if (!e_file) {
    perror(file_path);
    exit(1);
  }
  
  if(read_type == E_READ_RAM) {
    if(fseek(e_file, 0, SEEK_END) != 0) {
        perror("fseek");
        exit(1);
    }

    long file_size = ftell(e_file);
    if(file_size < 0) {
        perror("ftell");
        exit(1);
    }

    rewind(e_file);

    e_file_buffer = malloc((size_t)file_size);
    if(!e_file_buffer) {
        perror("malloc");
        exit(1);
    }

    if(fread(e_file_buffer, 1, (size_t)file_size, e_file) != (size_t)file_size) {
        perror("fread");
        exit(1);
    }

    bgReaderCreate(&e_reader, e_ram_read_cb);
} else {
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

/**
 * 実験終了
 */
static void e_end(void) {
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

/**
 * ノードコンテキスト、ウェイコンテキストの、重みをすべて0にする
 */
static void e_weight_zero(void) {
  for(bgIdx_t i = 0; i < e_wc.dim; i++) {
    bgWayContextSetWeight(&e_wc, i, 0);
  }
  for(bgIdx_t i = 0; i < e_nc.dim; i++) {
    bgNodeContextSetWeight(&e_nc, i, 0);
  }
}

/**
 * ノードコンテキスト、ウェイコンテキストの、重みをターミナルから受け取る
 */
static void e_weight_input(void) {
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

/**
 * ノードの指標番号を、ターミナルから受け取る
 */
static bgIdx_t e_node_ind_select(void) {
  bgIdx_t ind_idx;

  for(bgIdx_t i = 0; i < e_nc.dim; i++) {
    printf("%u: %s\n", i, bgNodeContextIndicatorName(&e_nc, i));
  }

  for(;;) {
    printf("select number = ");
    scanf("%u", &ind_idx);

    if(ind_idx < e_nc.dim) break;
    printf("invalid node indicator number !\n");
  }

  return ind_idx;
}

/**
 * ウェイの指標番号を、ターミナルから受け取る
 */
static bgIdx_t e_way_ind_select(void) {
  bgIdx_t ind_idx;

  for(bgIdx_t i = 0; i < e_wc.dim; i++) {
    printf("%u: %s\n", i, bgWayContextIndicatorName(&e_wc, i));
  }

  for(;;) {
    printf("select number = ");
    scanf("%u", &ind_idx);

    if(ind_idx < e_wc.dim) break;
    printf("invalid way indicator number !\n");
  }

  return ind_idx;
}

/**
 * 経路探索
 */
static void e_astar(bgIdx_t start_node_idx, bgIdx_t goal_node_idx, bgCost_t heuristic_weight) {
  bgAStarVisitedClear(&e_visited);
  bgAStarHeapClear(&e_heap);
  bgPathClear(&e_path);
  
  bgError_t err = bgAStar(
    &e_reader, &e_header,
    &e_nc, &e_wc,
    &e_node, &e_way,
    &e_visited, &e_heap,
    start_node_idx, goal_node_idx,
    heuristic_weight,
    &e_path
  );

  if(err != BG_ERROR_OK) {
    fprintf(stderr, "e_astar: failed.");
    exit(1);
  }
}


/**
 * 指標名からノード指標番号を取得
 */
static bgIdx_t e_node_ind_find(const char *name) {
  for(bgIdx_t i = 0; i < e_nc.dim; i++) {
    if(strcmp(bgNodeContextIndicatorName(&e_nc, i), name) == 0) {
      return i;
    }
  }
  fprintf(stderr, "node indicator not found: %s\n", name);
  exit(1);
}

/**
 * 指標名からウェイ指標番号を取得
 */
static bgIdx_t e_way_ind_find(const char *name) {
  for(bgIdx_t i = 0; i < e_wc.dim; i++) {
    if(strcmp(bgWayContextIndicatorName(&e_wc, i), name) == 0) {
      return i;
    }
  }
  fprintf(stderr, "way indicator not found: %s\n", name);
  exit(1);
}

#endif
