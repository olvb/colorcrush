#include <stdlib.h>
#include <stddef.h>
#include "pool.h"

#define POOL_BLOCK_SIZE 100000

struct PoolBlock {
    // block precedent ou NULL, permet de tout desallouer
    struct PoolBlock *prev;
    // adresse de debut du block
    Node *begin;
    // adresse de la portion non utilisee
    Node *next;
    // adresse de fin (evite le recalcul)
    Node *end;
};

/** Allocates and inits @block */
static void pool_block_init(PoolBlock *block, PoolBlock *prev) {
    // noeuds initialises a 0
    block->begin = calloc(POOL_BLOCK_SIZE, sizeof(Node));

    block->next = block->begin;
    block->end = block->begin + POOL_BLOCK_SIZE;
    // stores pointer to previously allocated block to perform chained freeing
    block->prev = prev;
}

void pool_init(Pool *pool) {
    pool->block = malloc(sizeof(PoolBlock));

    pool_block_init(pool->block, NULL);
}

Node *pool_next(Pool *pool) {
    // if current block if fully used, alloc new block
    if (pool->block->next == pool->block->end) {
        PoolBlock *new_block = malloc(sizeof(PoolBlock));
        pool_block_init(new_block, pool->block);
        pool->block = new_block;
    }
    return (pool->block->next)++;
}

void pool_clear(Pool *pool) {
    PoolBlock *block = pool->block;
    // walk up the chain of allocated blocks
    while (block != NULL) {
        PoolBlock *prev = pool->block;
        prev = block->prev;
        free(block->begin);
        free(block);
        block = prev;
    }
    pool->block = NULL;
}
