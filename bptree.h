// bptree.h

#ifndef BPTREE_H
#define BPTREE_H

#include <stddef.h>
#include "record.h"        // <<< Record tipi için

// B+ tree order
#define BP_DEGREE 4

extern size_t g_split_count;
extern size_t g_memory_usage_bytes;

typedef struct UniListNode {
    char *university;
    double score;
    struct UniListNode *next;
} UniListNode;

typedef struct BPNode {
    int is_leaf;
    int num_keys;
    char *keys[BP_DEGREE - 1];
    struct BPNode *children[BP_DEGREE + 1];
    struct BPNode *next, *prev;
    UniListNode *ulist[BP_DEGREE - 1];
} BPNode;

BPNode *create_node(int is_leaf);
void insert_sequential(BPNode **root, Record *rec);
void bulk_load(BPNode **root, Record *recs, size_t n);
UniListNode *search(BPNode *root, const char *dept, int k);
int tree_height(BPNode *root);
void destroy_tree(BPNode *root);

#endif // BPTREE_H
