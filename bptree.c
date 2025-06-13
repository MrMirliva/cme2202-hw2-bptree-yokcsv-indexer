// bptree.c

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "bptree.h"

// Helper to duplicate a key string
static char *key_dup(const char *s) {
    if (!s) return NULL;
    char *d = malloc(strlen(s) + 1);
    return d ? strcpy(d, s) : NULL;
}

BPNode *create_node(int is_leaf) {
    BPNode *node = malloc(sizeof(BPNode));
    if (!node) return NULL;
    node->is_leaf = is_leaf;
    node->num_keys = 0;
    node->next = NULL;
    node->prev = NULL;
    // Anahtar ve ikincil liste göstericilerini NULL’la
    for (int i = 0; i < BP_DEGREE - 1; i++) {
        node->keys[i]  = NULL;
        node->ulist[i] = NULL;
    }
    // Çocuk işaretçilerini NULL’la
    for (int i = 0; i < BP_DEGREE + 1; i++) {
        node->children[i] = NULL;
    }
    return node;
}

void split_node(BPNode *parent, int i) {
    BPNode *child    = parent->children[i];
    BPNode *new_node = create_node(child->is_leaf);
    int orig = child->num_keys;
    int t    = orig / 2;

    if (child->is_leaf) {
        // Yaprak bölme
        new_node->num_keys = orig - t;
        for (int j = 0; j < new_node->num_keys; ++j) {
            new_node->keys[j]  = child->keys[j + t];
            new_node->ulist[j] = child->ulist[j + t];
            child->keys[j + t]  = NULL;
            child->ulist[j + t] = NULL;
        }
        child->num_keys = t;

        // Çift yönlü leaf zinciri
        new_node->next = child->next;
        new_node->prev = child;
        if (child->next) child->next->prev = new_node;
        child->next = new_node;

        // Parent’ın children ve keys dizilerini kaydır, ekle
        for (int j = parent->num_keys; j > i; --j)
            parent->children[j + 1] = parent->children[j];
        parent->children[i + 1] = new_node;

        for (int j = parent->num_keys; j > i; --j)
            parent->keys[j] = parent->keys[j - 1];
        parent->keys[i] = key_dup(new_node->keys[0]);
        parent->num_keys++;
    } else {
        // İç düğüm bölme
        char *promoted = child->keys[t];

        // Sağ düğüme kalan anahtarları ve çocuk işaretçilerini taşı
        new_node->num_keys = orig - t - 1;
        for (int j = 0; j < new_node->num_keys; ++j) {
            new_node->keys[j]     = child->keys[j + t + 1];
            new_node->children[j] = child->children[j + t + 1];
            child->keys[j + t + 1]     = NULL;
            child->children[j + t + 1] = NULL;
        }
        // Son (t+1). çocuk da new_node’a geçmeli
        new_node->children[new_node->num_keys] = child->children[orig];
        child->children[orig]                  = NULL;

        child->num_keys = t;

        // Parent’ın children ve keys dizilerini kaydır, ekle
        for (int j = parent->num_keys; j > i; --j) {
            parent->children[j+1] = parent->children[j];
            parent->keys[j]       = parent->keys[j - 1];
        }
        parent->children[i + 1] = new_node;
        parent->keys[i]         = promoted;
        parent->num_keys++;
    }
}

// String duplication helper
static char *str_dup(const char *s) {
    if (!s) return NULL;
    char *d = malloc(strlen(s) + 1);
    return d ? strcpy(d, s) : NULL;
}

/**
 * Insert a Record into a node that is guaranteed not full.
 */
static void insert_nonfull(BPNode *node, Record *rec) {
    if (!node->is_leaf) {
        // İç düğüm: uygun çocuğa in
        int pos = 0;
        while (pos < node->num_keys &&
               strcmp(rec->department, node->keys[pos]) >= 0)
            pos++;
        BPNode *child = node->children[pos];
        if (child->num_keys == BP_DEGREE - 1) {
            split_node(node, pos);
            if (strcmp(rec->department, node->keys[pos]) > 0)
                pos++;
        }
        insert_nonfull(node->children[pos], rec);
    } else {
        // Yaprak düğüm ekleme
        int pos = 0;
        while (pos < node->num_keys &&
               strcmp(node->keys[pos], rec->department) < 0)
            pos++;
        // Aynı bölüm varsa listeye ekle
        if (pos < node->num_keys &&
            strcmp(node->keys[pos], rec->department) == 0) {
            UniListNode *newUL = malloc(sizeof(UniListNode));
            newUL->university = str_dup(rec->university);
            newUL->score      = rec->score;
            UniListNode **pp  = &node->ulist[pos];
            while (*pp && (*pp)->score > rec->score)
                pp = &(*pp)->next;
            newUL->next = *pp;
            *pp         = newUL;
        }
        // Yeni bölümse anahtar ve liste düğümü ekle
        else {
            for (int j = node->num_keys; j > pos; j--) {
                node->keys[j]  = node->keys[j - 1];
                node->ulist[j] = node->ulist[j - 1];
            }
            node->keys[pos] = str_dup(rec->department);
            UniListNode *newUL = malloc(sizeof(UniListNode));
            newUL->university  = str_dup(rec->university);
            newUL->score       = rec->score;
            newUL->next        = NULL;
            node->ulist[pos]   = newUL;
            node->num_keys++;
        }
    }
}

/**
 * Main insertion entry: handles root split if necessary,
 * then delegates to insert_nonfull().
 */
void insert_sequential(BPNode **root, Record *rec) {
    if (*root == NULL) {
        *root = create_node(1);
    }
    BPNode *r = *root;
    if (r->num_keys == BP_DEGREE - 1) {
        BPNode *s = create_node(0);
        s->children[0] = r;
        *root = s;
        split_node(s, 0);
        insert_nonfull(s, rec);
    } else {
        insert_nonfull(r, rec);
    }
}

UniListNode *search(BPNode *root, const char *department, int k) {
    BPNode *cur = root;
    // 1) İç düğümlerde uygun çocuğa in
    while (cur && !cur->is_leaf) {
        int pos = 0;
        while (pos < cur->num_keys &&
               strcmp(department, cur->keys[pos]) >= 0)
            pos++;
        cur = cur->children[pos];
    }
    if (!cur) return NULL;

    // 2) Yaprakta bölüm anahtarını bul
    int pos = 0;
    while (pos < cur->num_keys &&
           strcmp(department, cur->keys[pos]) != 0)
        pos++;
    if (pos == cur->num_keys) return NULL;

    // 3) İkincil listeyi k kez ileri sar
    UniListNode *ul = cur->ulist[pos];
    while (ul && --k > 0)
        ul = ul->next;
    return ul;
}

void destroy_tree(BPNode *node) {
    if (!node) return;
    if (!node->is_leaf) {
        for (int i = 0; i <= node->num_keys; i++)
            destroy_tree(node->children[i]);
    } else {
        for (int i = 0; i < node->num_keys; i++) {
            UniListNode *u = node->ulist[i];
            while (u) {
                UniListNode *tmp = u;
                u = u->next;
                free(tmp->university);
                free(tmp);
            }
        }
    }
    for (int i = 0; i < node->num_keys; i++)
        free(node->keys[i]);
    free(node);
}
