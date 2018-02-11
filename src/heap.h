#pragma once
#include <stddef.h>
#include "node.h"

typedef struct Heap {
    size_t size;
    Node **nodes;
} Heap;

void heap_init(Heap *heap, size_t max_size);
void heap_clear(Heap *heap);
void heap_push(Heap *heap, Node *node);
Node *heap_pop(Heap *heap);


