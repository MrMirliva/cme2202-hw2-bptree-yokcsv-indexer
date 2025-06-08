// bptree.h

#ifndef BPTREE_H
#define BPTREE_H

#include <stddef.h>
#include "record.h"

// B+ tree degree (max keys per node = BP_DEGREE–1, max child pointers per node = BP_DEGREE+1)
#define BP_DEGREE 4

// Secondary index: sorted linked list of universities in a leaf
typedef struct UniListNode {
    char *university;
    double score;
    struct UniListNode *next;
} UniListNode;

// B+ tree node
typedef struct BPNode {
    int is_leaf;                             // 1 if leaf, 0 otherwise
    int num_keys;                            // current number of keys
    char *keys[BP_DEGREE - 1];               // up to BP_DEGREE–1 department names
    struct BPNode *children[BP_DEGREE + 1];  // up to BP_DEGREE+1 child pointers
    struct BPNode *next;                     // leaf-to-leaf linked list
    UniListNode *ulist[BP_DEGREE - 1];       // for each key in a leaf, pointer to its UniList
} BPNode;

// Create a new node (leaf if is_leaf==1, else internal)
BPNode *create_node(int is_leaf);

// Insert a record into the tree sequentially (with splitting as needed)
void insert_sequential(BPNode **root, Record *rec);

// Search for the k-th university in the given department (1-based index)
// Returns NULL if department not found or fewer than k entries
UniListNode *search(BPNode *root, const char *department, int k);

void destroy_tree(BPNode *root);

#endif // BPTREE_H
