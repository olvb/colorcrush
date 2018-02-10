#include <stddef.h>
#include <assert.h>
#include "node.h"

bool node_is_leaf_parent(Node *node) {
    for (int i = 0; i < 8; i++) {
        if (node->children[i] != NULL) {
            if (node->children[i]->is_leaf) {
                return true;
            }
        }
    }
    return false;
}

uint32_t node_count_leaves(Node *node) {
    assert(node_is_leaf_parent(node));

    uint32_t count = 0;
    for (int i = 0; i < 8; i++) {
        if (node->children[i] == NULL) {
            continue;
        }

        if (node->children[i]->is_leaf) {
            count++;
        } else {
            count+= node_count_leaves(node->children[i]);
        }
    }

    return count;
}

unsigned int node_reduce(Node *node) {
    assert(node_is_leaf_parent(node));

    unsigned int removed_leaves_count = 0;
    for (int i = 0; i < 8; i++) {
        if (node->children[i] == NULL) {
            continue;
        }

        Node *child_node = node->children[i];
        assert(child_node->is_leaf);
    
        node->r += child_node->r;
        node->g += child_node->g;
        node->b += child_node->b;
        node->children[i] = NULL;

        removed_leaves_count++;
    }
    node->is_leaf = true;
    assert(removed_leaves_count > 0 && removed_leaves_count <= 8);
    
    return removed_leaves_count - 1;
}
