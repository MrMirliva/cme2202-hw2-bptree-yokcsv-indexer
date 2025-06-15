// Bulk_Load.c

#include "Bulk_Load.h"
#include <stdio.h>
#include <stdlib.h>

Node* root = NULL;

Node* createNode(int isLeaf) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->isLeaf = isLeaf;
    newNode->numKeys = 0;
    newNode->next = NULL;
    for (int i = 0; i < ORDER; i++) {
        newNode->children[i] = NULL;
    }
    return newNode;
}

void printTree(Node* node, int level) {
    if (!node) return;
    printf("Level %d [", level);
    for (int i = 0; i < node->numKeys; i++) {
        printf("%d", node->keys[i]);
        if (i != node->numKeys - 1) printf(" | ");
    }
    printf("]\n");

    if (!node->isLeaf) {
        for (int i = 0; i <= node->numKeys; i++) {
            printTree(node->children[i], level + 1);
        }
    }
}
