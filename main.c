// main.c

#include <stdio.h>
#include "record.h"
#include "bptree.h"

int main(void) {
    // 1) Kayıtları oku
    size_t n;
    Record *recs = read_records("yok_atlas.csv", &n);
    if (!recs) return 1;
    printf("Read %zu records from yok_atlas.csv\n", n);

    // 2) Ağacı kur
    BPNode *root = NULL;
    for (size_t i = 0; i < n; i++) {
        insert_sequential(&root, &recs[i]);
    }
    printf("Tree built. Root num_keys = %d\n", root->num_keys);

    // 3) Örnek aramalar
    const char *dept = "Gastronomi ve Mutfak Sanatlari";
    for (int k = 1; k <= 2; k++) {
        UniListNode *u = search(root, dept, k);
        if (u) {
            printf("%d%s university in \"%s\": %s (%.2f)\n",
                   k, k==1?"st":(k==2?"nd":"th"),
                   dept, u->university, u->score);
        } else {
            printf("%d%s university in \"%s\": not found\n", k,
                   k==1?"st":(k==2?"nd":"th"), dept);
        }
    }

    // 4) Temizlik
    destroy_tree(root);
    free_records(recs, n);
    return 0;
}
