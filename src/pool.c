#include <stdlib.h>
#include <stddef.h>
#include "pool.h"

#define POOL_BLOCK_SIZE 100000

struct PoolBlock {
    /** previous block or NULL if block is first */
    struct PoolBlock *prev;
    /** start of memory chunk */
    Node *begin;
    /** position of remaining memory */
    Node *next;
    /** end of memory chunk */
    Node *end;
};

static void pool_block_init(PoolBlock *block, PoolBlock *prev) {
    // calloc: init nodes with 0s
    block->begin = calloc(POOL_BLOCK_SIZE, sizeof(Node));

    block->next = block->begin;
    block->end = block->begin + POOL_BLOCK_SIZE;
    // stores pointer to previously allocated block to perform chained freeing
    block->prev = prev;
}

void pool_init(Pool *pool) {
    *pool = malloc(sizeof(PoolBlock));

    pool_block_init(*pool, NULL);
}

Node *pool_next(Pool *pool) {
    // if current block if fully used, alloc new block
    if ((*pool)->next == (*pool)->end) {
        PoolBlock *new_block = malloc(sizeof(PoolBlock));
        pool_block_init(new_block, (*pool));
        (*pool) = new_block;
    }
    return ((*pool)->next)++;
}

void pool_clear(Pool *pool) {
    PoolBlock *block = (*pool);
    // walk up the chain of allocated blocks
    while (block != NULL) {
        PoolBlock *prev = (*pool);
        prev = block->prev;
        free(block->begin);
        free(block);
        block = prev;
    }
    (*pool) = NULL;
}
