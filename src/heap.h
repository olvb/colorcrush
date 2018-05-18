#pragma once
#include "node.h"
#include <stddef.h>

typedef struct ccrush_heap_t {
    size_t size;
    ccrush_node_t **nodes;
} ccrush_heap_t;

void ccrush_heap_init(ccrush_heap_t *heap, size_t max_size);
void ccrush_heap_clear(ccrush_heap_t *heap);
void ccrush_heap_push(ccrush_heap_t *heap, ccrush_node_t *node);
ccrush_node_t *ccrush_heap_pop(ccrush_heap_t *heap);
void ccrush_heap_fill(ccrush_heap_t *heap, ccrush_node_t *node);
