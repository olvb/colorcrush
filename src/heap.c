#include <assert.h>
#include <stdlib.h>

#include "heap.h"

void heap_init(heap_t *heap, size_t max_size) {
    heap->nodes = malloc(sizeof(node_t) * max_size);
    heap->size = 0;
}

void heap_clear(heap_t *heap) {
    free(heap->nodes);
    heap->nodes = NULL;
}

static int node_cmp(node_t *lhs, node_t *rhs) {
    if (lhs->error > rhs->error) {
        return +1;
    }
    if (lhs->error < rhs->error) {
        return -1;
    }

    if (lhs->pixels_count > rhs->pixels_count) {
        return -1;
    }
    if (lhs->pixels_count < rhs->pixels_count) {
        return +1;
    }

    return 0;
}

void heap_push(heap_t *heap, node_t *node) {
    int index = heap->size;
    heap->size++;
    while (index != 0) {
        unsigned int parent_index = (index - 1) / 2;
        if (node_cmp(heap->nodes[parent_index], node) <= 0) {
            break;
        }

        heap->nodes[index] = heap->nodes[parent_index];
        index = parent_index;
    }

    heap->nodes[index] = node;
}

node_t *heap_pop(heap_t *heap) {
    assert(heap->size > 0);

    node_t *top_node = heap->nodes[0];
    if (heap->size == 1) {
        heap->size = 0;
        return top_node;
    }

    heap->size--;
    node_t *moved_node = heap->nodes[heap->size];
    int index = 0;
    int child_index = 1;
    while (child_index < heap->size) {
        // pick smallest of the 2 children
        if (child_index + 1 < heap->size && node_cmp(heap->nodes[child_index], heap->nodes[child_index + 1]) >= 0) {
            child_index++;
        }

        if (node_cmp(heap->nodes[child_index], moved_node) > 0) {
            break;
        }

        heap->nodes[index] = heap->nodes[child_index];
        index = child_index;
        child_index = index * 2 + 1;
    }

    heap->nodes[index] = moved_node;
    return top_node;
}
