#include <beltgraph.h>

void bgReaderSeek(bgReader_t *reader, bgOffset_t pos) {
  reader->pos = pos;
}

void bgReaderRead(bgReader_t *reader, void *dest, bgSize_t size) {
  reader->cb(reader->pos, dest, size);
  reader->pos += size;
}
