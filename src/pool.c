#include <stddef.h>
#include <stdlib.h>

#include "pool.h"

#define POOL_BLOCK_SIZE 100000

struct pool_block_t {
    /** previous block or NULL if block is first */
    struct pool_block_t *prev;
    /** start of memory chunk */
    node_t *begin;
    /** position of remaining memory */
    node_t *next;
    /** end of memory chunk */
    node_t *end;
};

static void pool_block_init(pool_block_t *block, pool_block_t *prev) {
    // calloc: init nodes with 0s
    block->begin = calloc(POOL_BLOCK_SIZE, sizeof(node_t));

    block->next = block->begin;
    block->end = block->begin + POOL_BLOCK_SIZE;
    // stores pointer to previously allocated block to perform chained freeing
    block->prev = prev;
}

void pool_init(pool_t *pool) {
    *pool = malloc(sizeof(pool_block_t));

    pool_block_init(*pool, NULL);
}

node_t *pool_next(pool_t *pool) {
    // if current block if fully used, alloc new block
    if ((*pool)->next == (*pool)->end) {
        pool_block_t *new_block = malloc(sizeof(pool_block_t));
        pool_block_init(new_block, (*pool));
        (*pool) = new_block;
    }
    return ((*pool)->next)++;
}

void pool_clear(pool_t *pool) {
    pool_block_t *block = (*pool);
    // walk up the chain of allocated blocks
    while (block != NULL) {
        pool_block_t *prev = (*pool);
        prev = block->prev;
        free(block->begin);
        free(block);
        block = prev;
    }
    (*pool) = NULL;
}
