#include <beltgraph.h>
#include <stdlib.h>
#include <string.h>

void bgNodeContextCreate(bgNodeContext_t *nc, const bgHeader_t *header) {
  const bgIdx_t dim = header->nc_dim;

  nc->dim = dim;
  nc->weights = malloc(dim * sizeof(bgWeight_t));

  nc->indicator_name_size = header->nc_ind_name_size;
  nc->indicator_names = malloc(header->nc_ind_name_size);
  nc->indicator_name_offs = malloc(dim * sizeof(bgOffset_t));
  
  nc->indicator_desc_size = header->nc_ind_desc_size;
  nc->indicator_descs = malloc(header->nc_ind_desc_size);
  nc->indicator_desc_offs = malloc(dim * sizeof(bgOffset_t));
}

void bgNodeContextDestroy(bgNodeContext_t *nc) {
  free(nc->weights);
  free(nc->indicator_names);
  free(nc->indicator_name_offs);
  free(nc->indicator_descs);
  free(nc->indicator_desc_offs);
}

void bgNodeContextFetch(bgNodeContext_t *nc, bgReader_t *reader, const bgHeader_t *header) {
  const bgIdx_t dim = header->nc_dim;

  bgReaderJump(reader, header->nc_pos);

  bgReaderRead(nc->indicator_names, nc->indicator_name_size, 1, reader);
  bgReaderRead(nc->indicator_name_offs, sizeof(bgOffset_t), dim, reader);
  
  bgReaderRead(nc->indicator_descs, nc->indicator_desc_size, 1, reader);
  bgReaderRead(nc->indicator_desc_offs, sizeof(bgOffset_t), dim, reader);
}

const char *bgNodeContextIndicatorName(const bgNodeContext_t *nc, bgIdx_t i) {
  return nc->indicator_names + nc->indicator_name_offs[i];
}

const char *bgNodeContextIndicatorDesc(const bgNodeContext_t *nc, bgIdx_t i) {
  return nc->indicator_descs + nc->indicator_desc_offs[i];
}

void bgNodeContextSetWeight(bgNodeContext_t *nc, bgIdx_t i, bgWeight_t weight) {
  nc->weights[i] = weight;
}

void bgNodeContextSetWeightPreset(bgNodeContext_t *nc, const bgWeight_t *weights) {
  memcpy(nc->weights, weights, nc->dim * sizeof(bgWeight_t));
}



void bgNodeCreate(bgNode_t *node, const bgHeader_t *header) {
  const bgIdx_t dim = header->nc_dim;

  node->indicators = malloc(dim * sizeof(bgIndicator_t));
}

void bgNodeDestroy(bgNode_t *node) {
  free(node->indicators);
}

void bgNodeFetch(bgNode_t *node, bgReader_t *reader, const bgHeader_t *header, bgIdx_t idx) {
  const bgIdx_t dim = header->nc_dim;
  node->idx = idx;

  bgReaderJump(
    reader,
    header->node_pos + header->node_stride * node->idx
  );

  bgReaderRead(&node->way_begin, sizeof(bgIdx_t), 1, reader);
  bgReaderRead(&node->way_end, sizeof(bgIdx_t), 1, reader);
  bgReaderRead(&node->lon, sizeof(double), 1, reader);
  bgReaderRead(&node->lat, sizeof(double), 1, reader);
  bgReaderRead(node->indicators, sizeof(bgIndicator_t), dim, reader);
}

bgCost_t bgNodeCost(const bgNode_t *node, const bgNodeContext_t *nc) {
  bgCost_t c = 0;

  for(bgIdx_t i = 0; i < nc->dim; i++) {
    if(nc->weights[i] > 0) {
      c += node->indicators[i] * nc->weights[i];
    }else {
      c += (1 - node->indicators[i]) * -nc->weights[i];
    }
  }

  c /= nc->dim;
  return c;
}



#ifdef GRAPHER
void bgNodeContextWrite(const bgNodeContext_t *nc, FILE *out, const bgHeader_t *header) {
  const bgIdx_t dim = header->nc_dim;

  fseek(
    out,
    header->nc_pos,
    SEEK_SET
  );

  /**
   * [!WARNING]
   * エンディアンを揃えた方が良いです
   */

  fwrite(nc->indicator_names, nc->indicator_name_size, 1, out);
  fwrite(nc->indicator_name_offs, sizeof(bgOffset_t), dim, out);
  
  fwrite(nc->indicator_descs, nc->indicator_desc_size, 1, out);
  fwrite(nc->indicator_desc_offs, sizeof(bgOffset_t), dim, out);
}

bgSize_t bgNodeContextByteCount(bgIdx_t nc_dim, bgSize_t nc_ind_name_size, bgSize_t nc_ind_desc_size) {
  bgSize_t size = 0;

  size += nc_ind_name_size;
  size += sizeof(bgOffset_t) * nc_dim;

  size += nc_ind_desc_size;
  size += sizeof(bgOffset_t) * nc_dim;

  return size;
}



void bgNodeWrite(const bgNode_t *node, FILE *out, const bgHeader_t *header) {
  const bgIdx_t dim = header->nc_dim;

  fseek(
    out,
    header->node_pos + header->node_stride * node->idx,
    SEEK_SET
  );

  fwrite(&node->way_begin, sizeof(bgIdx_t), 1, out);
  fwrite(&node->way_end, sizeof(bgIdx_t), 1, out);
  fwrite(&node->lon, sizeof(double), 1, out);
  fwrite(&node->lat, sizeof(double), 1, out);
  fwrite(node->indicators, sizeof(bgIndicator_t), dim, out);
}

bgSize_t bgNodeByteCount(bgIdx_t nc_dim) {
  bgSize_t size = 0;

  size += sizeof(bgIdx_t);
  size += sizeof(bgIdx_t);
  size += sizeof(double);
  size += sizeof(double);
  size += sizeof(bgIndicator_t) * nc_dim;

  return size;
}
#endif
