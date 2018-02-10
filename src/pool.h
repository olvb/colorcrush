#pragma once
#include "node.h"

typedef struct PoolBlock PoolBlock;
/** Memory pool handling allocation and freeing of Nodes */
typedef PoolBlock* Pool;

void pool_init(Pool *pool);
/** @returns pointer on allocated and ready to use Node */
Node *pool_next(Pool *pool);
void pool_clear(Pool *pool);
