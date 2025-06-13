// bptree.h

#ifndef BPTREE_H
#define BPTREE_H

#include <stddef.h>
#include "record.h"

// B+ tree degree: max child pointers per node = BP_DEGREE
//                       max keys per node      = BP_DEGREE-1
#define BP_DEGREE 4

// Secondary index: sorted linked list of universities in a leaf
typedef struct UniListNode {
    char *university;
    double score;
    struct UniListNode *next;
} UniListNode;

// B+ tree node
typedef struct BPNode {
    int is_leaf;                            // 1 if leaf, 0 otherwise
    int num_keys;                           // current number of keys
    char *keys[BP_DEGREE - 1];              // up to BP_DEGREE-1 department names
    struct BPNode *children[BP_DEGREE];     // up to BP_DEGREE child pointers
    struct BPNode *next;                     // leaf-to-leaf linked list
    struct BPNode *prev;                     // backward link in leaf list

    UniListNode *ulist[BP_DEGREE - 1];      // for each key in leaf, pointer to its UniList
} BPNode;

// --- Global statistics (splits & memory usage) --------------------------------
extern size_t g_split_count;
extern size_t g_memory_usage_bytes;

// Create a new node (leaf if is_leaf==1, else internal)
BPNode *create_node(int is_leaf);

// Insert a record into the tree sequentially (with splitting)
void insert_sequential(BPNode **root, Record *rec);

// Bulk‐load from sorted array of keys
void bulk_load(int keys[], int n);

// Search for the k-th university in the given department (1-based index)
// Returns NULL if not found or fewer than k entries
UniListNode *search(BPNode *root, const char *department, int k);

// Compute the height (number of levels) of the B+ tree
int tree_height(BPNode *root);

// Free all nodes and secondary lists
void destroy_tree(BPNode *root);

#endif // BPTREE_H
