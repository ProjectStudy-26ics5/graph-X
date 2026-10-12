#ifndef BELTGRAPH_READER_H
#define BELTGRAPH_READER_H

#include <beltgraph/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*bgReaderCallback)(bgOffset_t pos, void *dest, bgSize_t size);

struct bgReader {
  bgOffset_t pos;
  bgReaderCallback cb;
};

void bgReaderSeek(bgReader_t *reader, bgOffset_t pos);
void bgReaderRead(bgReader_t *reader, void *dest, bgSize_t size);

#ifdef __cplusplus
}
#endif

#endif
