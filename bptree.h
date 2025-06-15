#ifndef BPTREE_H
#define BPTREE_H

#include <stddef.h>
#include "record.h"        // Record tanımı için

// Komut satırından atanacak B+ ağacı derecesi (order)
extern int bp_degree;

// Sayaçlar
extern size_t g_split_count;
extern size_t g_memory_usage_bytes;

// Üniversite listesi düğümü
typedef struct UniListNode {
    char *university;
    double score;
    struct UniListNode *next;
} UniListNode;

// B+ ağacı düğümü
typedef struct BPNode {
    int is_leaf;               // 1: yaprak, 0: iç düğüm
    int num_keys;              // şu anda tutulan anahtar sayısı

    // Aşağıdakiler dinamik olarak bp_degree’a göre malloc ile ayrılacak
    char          **keys;      // (bp_degree - 1) elemanlı
    struct BPNode **children;  // (bp_degree + 1) elemanlı
    UniListNode   **ulist;     // (bp_degree - 1) elemanlı

    struct BPNode *next, *prev; // yapraklar arası çift yönlü zincir
} BPNode;

// Düğüm oluşturma: is_leaf bayrağına göre alan tahsis eder ve
// keys, children, ulist dizilerini bp_degree’a uygun olarak malloc’lar.
BPNode *create_node(int is_leaf);

// Sıralı ekleme yöntemi
void insert_sequential(BPNode **root, Record *rec);

// Bulk loading yöntemi (external sort’lu)
void bulk_load(BPNode **root, Record *recs, size_t n);

// Arama: dept departmanı için k’ıncı üniversitenin listesi
UniListNode *search(BPNode *root, const char *dept, int k);

// Ağacın yüksekliğini hesaplar
int tree_height(BPNode *root);

// Ağacı ve bağlı tüm bellek bloklarını serbest bırakır
void destroy_tree(BPNode *root);

#endif // BPTREE_H
