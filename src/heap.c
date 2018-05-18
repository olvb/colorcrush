#include "heap.h"
#include <assert.h>
#include <stdlib.h>

static int ccrush_heap_cmp_nodes(ccrush_node_t *lhs, ccrush_node_t *rhs);

void ccrush_heap_init(ccrush_heap_t *heap, size_t max_size) {
    heap->nodes = malloc(sizeof(ccrush_node_t) * max_size);
    heap->size = 0;
}

void ccrush_heap_clear(ccrush_heap_t *heap) {
    free(heap->nodes);
    heap->nodes = NULL;
}

static int ccrush_heap_cmp_nodes(ccrush_node_t *lhs, ccrush_node_t *rhs) {
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

void ccrush_heap_push(ccrush_heap_t *heap, ccrush_node_t *node) {
    int index = heap->size;
    heap->size++;
    while (index != 0) {
        unsigned int parent_index = (index - 1) / 2;
        if (ccrush_heap_cmp_nodes(heap->nodes[parent_index], node) <= 0) {
            break;
        }

        heap->nodes[index] = heap->nodes[parent_index];
        index = parent_index;
    }

    heap->nodes[index] = node;
}

ccrush_node_t *ccrush_heap_pop(ccrush_heap_t *heap) {
    assert(heap->size > 0);

    ccrush_node_t *top_node = heap->nodes[0];
    if (heap->size == 1) {
        heap->size = 0;
        return top_node;
    }

    heap->size--;
    ccrush_node_t *moved_node = heap->nodes[heap->size];
    int index = 0;
    int child_index = 1;
    while (child_index < heap->size) {
        // pick smallest of the 2 children
        if (child_index + 1 < heap->size && ccrush_heap_cmp_nodes(heap->nodes[child_index], heap->nodes[child_index + 1]) >= 0) {
            child_index++;
        }

        if (ccrush_heap_cmp_nodes(heap->nodes[child_index], moved_node) > 0) {
            break;
        }

        heap->nodes[index] = heap->nodes[child_index];
        index = child_index;
        child_index = index * 2 + 1;
    }

    heap->nodes[index] = moved_node;
    return top_node;
}

void ccrush_heap_fill(ccrush_heap_t *heap, ccrush_node_t *node) {
    assert(!node->is_leaf);

    ccrush_heap_push(heap, node);

    for (int i = 0; i < 8; i++) {
        if (node->children[i] == NULL) {
            continue;
        }

        ccrush_node_t *child = node->children[i];
        if (!child->is_leaf) {
            ccrush_heap_fill(heap, child);
        }
    }
}
