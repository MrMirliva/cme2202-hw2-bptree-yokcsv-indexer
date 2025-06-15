// bptree.c

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "bptree.h"
#include "record.h"   // Record tipi için

// Yardımcı struct: her departmana karşılık gelen uni-list başı
typedef struct {
    const char *dept;
    UniListNode *ulist;
} DeptEntry;

size_t g_split_count = 0;
size_t g_memory_usage_bytes = 0;

// String duplicate + bellek sayaç
static char *key_dup(const char *s) {
    if (!s) return NULL;
    size_t sz = strlen(s) + 1;
    char *d = malloc(sz);
    if (!d) { perror("malloc"); exit(1); }
    memcpy(d, s, sz);
    g_memory_usage_bytes += sz;
    return d;
}

// bptree.c içinde, diğer static fonksiyonların yanına
static BPNode *find_leaf(BPNode *root, const char *key) {
    BPNode *c = root;
    int level = 0;
    while (c && !c->is_leaf) {
        // 1) Bu seviyedeki anahtarları bas
        printf("[DEBUG] Level %d keys:", level);
        for (int i = 0; i < c->num_keys; i++)
            printf(" '%s'", c->keys[i]);
        printf("\n");

        // 2) Hangi child’a iniyor?
        int i = 0;
        while (i < c->num_keys && strcmp(key, c->keys[i]) >= 0) i++;
        printf("[DEBUG] Level %d choose child index = %d\n", level, i);

        c = c->children[i];
        level++;
    }
    printf("[DEBUG] Reached leaf at level %d with num_keys=%d\n",
           level, c ? c->num_keys : -1);
    return c;
}


BPNode *create_node(int is_leaf) {
    BPNode *n = malloc(sizeof *n);
    if (!n) { perror("malloc"); exit(1); }
    g_memory_usage_bytes += sizeof *n;
    n->is_leaf = is_leaf;
    n->num_keys = 0;
    n->next = n->prev = NULL;
    for (int i = 0; i < BP_DEGREE - 1; i++) {
        n->keys[i]  = NULL;
        n->ulist[i] = NULL;
    }
    for (int i = 0; i < BP_DEGREE + 1; i++) {
        n->children[i] = NULL;
    }
    return n;
}

// node->children[idx] düğümünü bölüp parent'a yeni sibling ekler
static void split_node(BPNode *parent, int idx) {
    g_split_count++;
    BPNode *child = parent->children[idx];
    BPNode *sib   = create_node(child->is_leaf);
    int t = child->num_keys / 2;

    if (child->is_leaf) {
        // leaf bölme
        sib->num_keys = child->num_keys - t;
        for (int j = 0; j < sib->num_keys; j++) {
            sib->keys[j]  = child->keys[j + t];
            sib->ulist[j] = child->ulist[j + t];
            child->keys[j + t]  = NULL;
            child->ulist[j + t] = NULL;
        }
        child->num_keys = t;
        // zincir
        sib->next = child->next;
        if (sib->next) sib->next->prev = sib;
        child->next = sib;
        sib->prev  = child;
        // parent güncelle
        for (int j = parent->num_keys; j > idx; j--)
            parent->children[j+1] = parent->children[j];
        parent->children[idx+1] = sib;
        for (int j = parent->num_keys; j > idx; j--)
            parent->keys[j] = parent->keys[j-1];
        parent->keys[idx] = key_dup(sib->keys[0]);
        parent->num_keys++;

    } else {
        // internal split
        char *mid = child->keys[t];
        sib->num_keys = child->num_keys - t - 1;
        for (int j = 0; j < sib->num_keys; j++) {
            sib->keys[j]      = child->keys[j + t + 1];
            sib->children[j]  = child->children[j + t + 1];
            child->keys[j + t + 1]     = NULL;
            child->children[j + t + 1] = NULL;
        }
        sib->children[sib->num_keys] = child->children[child->num_keys];
        child->num_keys = t;
        for (int j = parent->num_keys; j > idx; j--) {
            parent->keys[j]       = parent->keys[j-1];
            parent->children[j+1] = parent->children[j];
        }
        parent->keys[idx]       = mid;
        parent->children[idx+1] = sib;
        parent->num_keys++;
    }
}

// insert_sequential içinde kullanılıyor
static void insert_nonfull(BPNode *n, Record *r) {
    if (!n->is_leaf) {
        int i = 0;
        while (i < n->num_keys && strcmp(r->department, n->keys[i]) >= 0) i++;
        BPNode *c = n->children[i];
        if (c->num_keys == BP_DEGREE-1) {
            split_node(n, i);
            if (strcmp(r->department, n->keys[i]) > 0) i++;
        }
        insert_nonfull(n->children[i], r);
    } else {
        int i = 0;
        while (i < n->num_keys && strcmp(n->keys[i], r->department) < 0) i++;
        if (i < n->num_keys && strcmp(n->keys[i], r->department) == 0) {
            // Mevcut departmana ekle
            UniListNode *u = malloc(sizeof *u);
            if (!u) { perror("malloc"); exit(1); }
            g_memory_usage_bytes += sizeof *u;
            u->university = key_dup(r->university);
            u->score      = r->score;
            UniListNode **pp = &n->ulist[i];
            while (*pp && (*pp)->score > r->score) pp = &(*pp)->next;
            u->next = *pp; *pp = u;
        } else {
            // Yeni departman anahtar ekle
            for (int j = n->num_keys; j > i; j--) {
                n->keys[j]  = n->keys[j-1];
                n->ulist[j] = n->ulist[j-1];
            }
            n->keys[i] = key_dup(r->department);
            UniListNode *u = malloc(sizeof *u);
            if (!u) { perror("malloc"); exit(1); }
            g_memory_usage_bytes += sizeof *u;
            u->university = key_dup(r->university);
            u->score      = r->score;
            u->next       = NULL;
            n->ulist[i]   = u;
            n->num_keys++;
        }
    }
}

void insert_sequential(BPNode **root, Record *r) {
    if (!*root) *root = create_node(1);
    BPNode *rt = *root;
    if (rt->num_keys == BP_DEGREE-1) {
        BPNode *s = create_node(0);
        s->children[0] = rt;
        *root = s;
        split_node(s, 0);
        insert_nonfull(s, r);
    } else {
        insert_nonfull(rt, r);
    }
}

void bulk_load(BPNode **root, Record *recs, size_t n) {
    // Reset counters
    g_split_count        = 0;
    g_memory_usage_bytes = 0;

    // 1) Destroy existing tree, if any
    if (*root) {
        destroy_tree(*root);
        *root = NULL;
    }

    // Phase 1: count distinct departments
    size_t D = 0;
    for (size_t i = 0; i < n; ) {
        D++;
        size_t j = i + 1;
        while (j < n && strcmp(recs[j].department, recs[i].department) == 0) j++;
        i = j;
    }
    DeptEntry *deps = malloc(D * sizeof *deps);
    if (!deps) { perror("malloc"); exit(1); }

    // Build DeptEntry array
    size_t di = 0;
    for (size_t i = 0; i < n; ) {
        deps[di].dept  = recs[i].department;
        deps[di].ulist = NULL;
        UniListNode *tail = NULL;
        size_t j = i;
        while (j < n && strcmp(recs[j].department, recs[i].department) == 0) {
            UniListNode *u = malloc(sizeof *u);
            if (!u) { perror("malloc"); exit(1); }
            g_memory_usage_bytes += sizeof *u;
            u->university = key_dup(recs[j].university);
            u->score      = recs[j].score;
            u->next       = NULL;
            if (!deps[di].ulist)
                deps[di].ulist = u;
            else
                tail->next = u;
            tail = u;
            j++;
        }
        di++;
        i = j;
    }

    // Phase 2: create leaf nodes
    size_t leafCap   = BP_DEGREE - 1;
    size_t leafCount = (D + leafCap - 1) / leafCap;
    BPNode **level   = malloc(leafCount * sizeof *level);
    if (!level) { perror("malloc"); exit(1); }

    for (size_t li = 0; li < leafCount; li++) {
        BPNode *leaf = create_node(1);
        size_t start = li * leafCap;
        size_t end   = start + leafCap;
        if (end > D) end = D;
        for (size_t k = start; k < end; k++) {
            size_t idx = k - start;
            leaf->keys[idx]  = key_dup(deps[k].dept);
            leaf->ulist[idx] = deps[k].ulist;
            leaf->num_keys++;
        }
        if (li > 0) {
            level[li-1]->next = leaf;
            leaf->prev        = level[li-1];
        }
        level[li] = leaf;
    }
    free(deps);

    // Phase 3: build internal nodes
    size_t lvlCount = leafCount;
    size_t group    = BP_DEGREE;
    while (lvlCount > 1) {
        size_t newCount  = (lvlCount + group - 1) / group;
        BPNode **nextLvl = malloc(newCount * sizeof *nextLvl);
        if (!nextLvl) { perror("malloc"); exit(1); }

        for (size_t i = 0; i < newCount; i++) {
            BPNode *node = create_node(0);
            size_t start = i * group;
            size_t end   = start + group;
            if (end > lvlCount) end = lvlCount;

            // 1) attach child pointers
            for (size_t j = start; j < end; j++) {
                node->children[j - start] = level[j];
            }

            // 2) for each split k, find the leftmost leaf under that child
            for (size_t k = 1; k < end - start; k++) {
                BPNode *c = level[start + k];
                while (!c->is_leaf) {
                    c = c->children[0];
                }
                node->keys[k - 1] = key_dup(c->keys[0]);
            }

            // 3) set number of keys
            node->num_keys = (int)(end - start) - 1;
            nextLvl[i]     = node;
        }

        free(level);
        level    = nextLvl;
        lvlCount = newCount;
    }

    // Phase 4: set new root
    *root = level[0];
    free(level);
}



UniListNode *search(BPNode *root, const char *dept, int k) {
    BPNode *c = root;
    while (c && !c->is_leaf) {
        int i=0;
        while (i<c->num_keys && strcmp(dept,c->keys[i])>=0) i++;
        c = c->children[i];
    }
    if (!c) return NULL;
    int i=0;
    while (i<c->num_keys && strcmp(dept,c->keys[i])!=0) i++;
    if (i==c->num_keys) return NULL;
    UniListNode *u = c->ulist[i];
    while (u && --k>0) u=u->next;
    return u;
}

int tree_height(BPNode *root) {
    int h=0;
    BPNode *c=root;
    while (c) {
        h++;
        if (c->is_leaf) break;
        c = c->children[0];
    }
    return h;
}

void destroy_tree(BPNode *n) {
    if (!n) return;
    if (!n->is_leaf) {
        for (int i=0; i<=n->num_keys; i++)
            destroy_tree(n->children[i]);
    } else {
        for (int i=0; i<n->num_keys; i++) {
            UniListNode *u = n->ulist[i];
            while (u) {
                UniListNode *t = u;
                u = u->next;
                free(t->university);
                free(t);
            }
        }
    }
    for (int i=0; i<n->num_keys; i++)
        free(n->keys[i]);
    free(n);
}