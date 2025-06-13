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

void bulkLoad(int keys[], int n) {
    int i, j;
    int numLeaves = (n + ORDER - 2) / (ORDER - 1);
    Node** leaves = (Node**)malloc(numLeaves * sizeof(Node*));

    // 1) Yaprak düğümlerini oluştur ve bağla
    for (i = 0; i < numLeaves; i++) {
        leaves[i] = createNode(1);
        for (j = 0; j < ORDER - 1 && (i * (ORDER - 1) + j) < n; j++) {
            leaves[i]->keys[j] = keys[i * (ORDER - 1) + j];
        }
        leaves[i]->numKeys = j;
        if (i > 0) {
            leaves[i - 1]->next = leaves[i];
        }
    }

    // 2) Üst seviye düğümleri bottom-up oluştur
    int levelSize = numLeaves;
    Node** currentLevel = leaves;
    while (levelSize > 1) {
        int newLevelSize = (levelSize + ORDER - 2) / (ORDER - 1);
        Node** newLevel = (Node**)malloc(newLevelSize * sizeof(Node*));

        for (i = 0; i < newLevelSize; i++) {
            newLevel[i] = createNode(0);
            for (j = 0; j < ORDER - 1 && (i * (ORDER - 1) + j) < levelSize; j++) {
                newLevel[i]->children[j] = currentLevel[i * (ORDER - 1) + j];
                if (j > 0) {
                    newLevel[i]->keys[j - 1] = currentLevel[i * (ORDER - 1) + j]->keys[0];
                }
            }
            // Son child pointer’ı da bağla
            newLevel[i]->children[j] =
                currentLevel[i * (ORDER - 1) + j - 1]->next;
            newLevel[i]->numKeys = j - 1;
        }

        free(currentLevel);
        currentLevel = newLevel;
        levelSize = newLevelSize;
    }

    root = currentLevel[0];
    free(currentLevel);
}
