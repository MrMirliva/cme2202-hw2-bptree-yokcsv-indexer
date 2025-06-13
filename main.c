#include <stdio.h>
#include <stdlib.h>      // ← burayı ekliyoruz
#include "external_sort.h"
#include "record.h"
#include "bptree.h"

int main(void) {
    printf("1) external_sort çağrılıyor...\n");
    char *sorted = external_sort("yok_atlas.csv");
    printf("1) external_sort bitti, dosya: %s\n", sorted);

    printf("2) read_records çağrılıyor...\n");
    size_t n;
    Record *recs = read_records(sorted, &n);
    printf("2) read_records tamamlandı, n = %zu, recs = %p\n", n, (void*)recs);
    if (!recs) return 1;

    printf("3) Ağaç inşasına geçiliyor...\n");
    BPNode *root = NULL;
    for (size_t i = 0; i < n; i++) {
        insert_sequential(&root, &recs[i]);
    }
    printf("3) Ağaç oluşturuldu, root->num_keys = %d\n", root->num_keys);

    // örnek arama
    UniListNode *u = search(root, "Gastronomi ve Mutfak Sanatlari", 2);
    if (u) printf("2. sıra üniversite: %s (%.2f)\n", u->university, u->score);
    else   printf("Arama bulunamadı.\n");

    free_records(recs, n);
    free(sorted);
    return 0;
}
