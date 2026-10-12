#include <beltgraph.h>
#include <stdlib.h>

void bgWayContextCreate(bgWayContext_t *wc, const bgHeader_t *header) {
  const bgIdx_t dim = header->wc_dim;

  wc->dim = dim;
  wc->weights = malloc(dim * sizeof(bgWeight_t));

  wc->indicator_name_size = header->wc_ind_name_size;
  wc->indicator_names = malloc(header->wc_ind_name_size);
  wc->indicator_name_offs = malloc(dim * sizeof(bgOffset_t));
  
  wc->indicator_desc_size = header->wc_ind_desc_size;
  wc->indicator_descs = malloc(header->wc_ind_desc_size);
  wc->indicator_desc_offs = malloc(dim * sizeof(bgOffset_t));
}

void bgWayContextDestroy(bgWayContext_t *wc) {
  free(wc->weights);
  free(wc->indicator_names);
  free(wc->indicator_name_offs);
  free(wc->indicator_descs);
  free(wc->indicator_desc_offs);
}



void bgWayCreate(bgWay_t *way, const bgHeader_t *header) {
  const bgIdx_t dim = header->wc_dim;

  way->indicators = malloc(dim * sizeof(bgIndicator_t));
}

void bgWayDestroy(bgWay_t *way) {
  free(way->indicators);
}



#ifdef GRAPHER
void bgWayContextWrite(const bgWayContext_t *wc, FILE *out, const bgHeader_t *header) {
  const bgIdx_t dim = header->wc_dim;

  fseek(
    out,
    header->wc_pos,
    SEEK_SET
  );

  /**
   * [!WARNING]
   * エンディアンを揃えた方が良いです
   */

  fwrite(wc->indicator_names, wc->indicator_name_size, 1, out);
  fwrite(wc->indicator_name_offs, sizeof(bgOffset_t), dim, out);
  
  fwrite(wc->indicator_descs, wc->indicator_desc_size, 1, out);
  fwrite(wc->indicator_desc_offs, sizeof(bgOffset_t), dim, out);
}

bgSize_t bgWayContextByteCount(bgIdx_t wc_dim, bgSize_t wc_ind_name_size, bgSize_t wc_ind_desc_size) {
  bgSize_t size = 0;

  size += wc_ind_name_size;
  size += sizeof(bgOffset_t) * wc_dim;

  size += wc_ind_desc_size;
  size += sizeof(bgOffset_t) * wc_dim;

  return size;
}



void bgWayWrite(const bgWay_t *way, FILE *out, const bgHeader_t *header) {
  const bgIdx_t dim = header->wc_dim;

  fseek(
    out,
    header->way_pos + header->way_stride * way->idx,
    SEEK_SET
  );

  fwrite(&way->to, sizeof(bgIdx_t), 1, out);
  fwrite(&way->length, sizeof(double), 1, out);
  fwrite(way->indicators, sizeof(bgIndicator_t), dim, out);
}

bgSize_t bgWayByteCount(bgIdx_t wc_dim) {
  bgSize_t size = 0;

  size += sizeof(bgIdx_t);
  size += sizeof(double);
  size += sizeof(bgIndicator_t) * wc_dim;
  
  return size;
}
#endif
