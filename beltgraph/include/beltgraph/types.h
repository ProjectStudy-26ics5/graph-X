#ifndef BELTGRAPH_TYPES_H
#define BELTGRAPH_TYPES_H

#include <stdint.h>
#include <stddef.h>
#ifdef GRAPHER
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef float bgIndicator_t;
typedef float bgWeight_t;
typedef float bgCost_t;

typedef uint64_t bgOffset_t;
typedef uint32_t bgIdx_t;
typedef uint64_t bgSize_t;

typedef struct bgHeader bgHeader_t;

typedef struct bgWay bgWay_t;
typedef struct bgWayContext bgWayContext_t;

typedef struct bgNode bgNode_t;
typedef struct bgNodeContext bgNodeContext_t;

#ifdef __cplusplus
}
#endif

#endif
