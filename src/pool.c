#include "pool.h"
#include <stddef.h>
#include <stdlib.h>

#define POOL_BLOCK_SIZE 100000

struct ccrush_poolblock_t {
    /** previous block or NULL if block is first */
    struct ccrush_poolblock_t *prev;
    /** start of memory chunk */
    ccrush_node_t *begin;
    /** position of remaining memory */
    ccrush_node_t *next;
    /** end of memory chunk */
    ccrush_node_t *end;
};

static void ccrush_poolblock_init(ccrush_poolblock_t *block, ccrush_poolblock_t *prev) {
    // calloc: init nodes with 0s
    block->begin = calloc(POOL_BLOCK_SIZE, sizeof(ccrush_node_t));

    block->next = block->begin;
    block->end = block->begin + POOL_BLOCK_SIZE;
    // stores pointer to previously allocated block to perform chained freeing
    block->prev = prev;
}

void ccrush_pool_init(ccrush_pool_t *pool) {
    *pool = malloc(sizeof(ccrush_poolblock_t));

    ccrush_poolblock_init(*pool, NULL);
}

ccrush_node_t *ccrush_pool_next(ccrush_pool_t *pool) {
    // if current block if fully used, alloc new block
    if ((*pool)->next == (*pool)->end) {
        ccrush_poolblock_t *new_block = malloc(sizeof(ccrush_poolblock_t));
        ccrush_poolblock_init(new_block, (*pool));
        (*pool) = new_block;
    }
    return ((*pool)->next)++;
}

void ccrush_pool_clear(ccrush_pool_t *pool) {
    ccrush_poolblock_t *block = (*pool);
    // walk up the chain of allocated blocks
    while (block != NULL) {
        ccrush_poolblock_t *prev = (*pool);
        prev = block->prev;
        free(block->begin);
        free(block);
        block = prev;
    }
    (*pool) = NULL;
}
