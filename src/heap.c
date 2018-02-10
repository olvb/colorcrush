#include <stdlib.h>
#include <assert.h>
#include "heap.h"

void heap_init(Heap *heap, size_t max_size) {
    heap->nodes = malloc(sizeof(Node) * max_size);
    heap->size = 0;
}

void heap_clear(Heap *heap) {
    free(heap->nodes);
    heap->nodes = NULL;
}

static int node_cmp(Node *lhs, Node *rhs) {
    if (lhs->pixels_count < rhs->pixels_count) {
        return -1;
    }
    if (lhs->pixels_count > rhs->pixels_count) {
        return +1;
    }
    return 0;
}

void heap_push(Heap *heap, Node *node) {
    assert(node_is_leaf_parent(node));

    int index = heap->size;
    heap->size++;
    int parent_index = (index - 1) >> 1;
    while(parent_index >= 0) {
        if (node_cmp(heap->nodes[parent_index], node) < 0) {
            break;
        }

        heap->nodes[index] = heap->nodes[parent_index];
        index = parent_index;
        parent_index = (index - 1) >> 1;
    }

    heap->nodes[index] = node;
}

Node *heap_pop(Heap *heap) {
    assert(heap->size > 0);

    Node *top_node = heap->nodes[0];
    if (heap->size == 1) {
        heap->size = 0;
        return top_node;
    }

    heap->size--;
    Node *moved_node = heap->nodes[heap->size];
    int index = 0;
    int child_index = 1;
    while (child_index < heap->size) {
        // pick smallest of the 2 children
        if (child_index + 1 < heap->size && node_cmp(heap->nodes[child_index], heap->nodes[child_index + 1]) > 0) {
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

void heap_fill(Heap *heap, Node *node, unsigned int depth) {
    if (depth == 1) {
        if (node_is_leaf_parent(node)) {
            heap_push(heap, node);
            return;
        } else {
            return;
       }
    }

    for (int i = 0; i < 8; i++) {
        if (node->children[i] == NULL) {
            continue;
        }
        heap_fill(heap, node->children[i], depth - 1);
    }
}
