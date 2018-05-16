#pragma once
#include <stddef.h>

#include "node.h"

typedef struct heap_t {
    size_t size;
    node_t **nodes;
} heap_t;

void heap_init(heap_t *heap, size_t max_size);
void heap_clear(heap_t *heap);
void heap_push(heap_t *heap, node_t *node);
node_t *heap_pop(heap_t *heap);
