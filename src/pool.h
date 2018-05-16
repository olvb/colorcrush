#pragma once
#include "node.h"

typedef struct pool_block_t pool_block_t;
/** Memory pool handling allocation and freeing of Nodes */
typedef pool_block_t *pool_t;

void pool_init(pool_t *pool);
/** @returns pointer on allocated and ready to use Node */
node_t *pool_next(pool_t *pool);
void pool_clear(pool_t *pool);
