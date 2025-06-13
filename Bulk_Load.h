// Bulk_Load.h

#ifndef BULK_LOAD_H
#define BULK_LOAD_H

#define ORDER 3

// B+ tree node used for bulk‐loading demo
typedef struct Node {
    int isLeaf;
    int numKeys;
    int keys[ORDER - 1];
    struct Node* children[ORDER];
    struct Node* next;
} Node;

// Global root pointer for the demo tree
extern Node* root;

// Allocate and initialize a new node (leaf if isLeaf=1)
Node* createNode(int isLeaf);

// Pretty‐print the tree, indenting by level
void printTree(Node* node, int level);

// Build a B+ tree bottom‐up from the sorted array 'keys' of length 'n'
void bulkLoad(int keys[], int n);

#endif // BULK_LOAD_H
