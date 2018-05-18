#pragma once
#include "node.h"

typedef struct ccrush_poolblock_t ccrush_poolblock_t;
/** Memory pool handling allocation and freeing of nodes */
typedef ccrush_poolblock_t *ccrush_pool_t;

void ccrush_pool_init(ccrush_pool_t *pool);
/** @returns pointer on allocated and ready to use node */
ccrush_node_t *ccrush_pool_next(ccrush_pool_t *pool);
void ccrush_pool_clear(ccrush_pool_t *pool);
