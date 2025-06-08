// main.c

#include <stdio.h>
#include <stdlib.h>
#include "record.h"
#include "bptree.h"

int main(void) {
    // 1) CSV’den tüm kayıtları oku
    size_t n;
    Record *recs = read_records("yok_atlas.csv", &n);
    if (!recs) {
        fprintf(stderr, "Error reading CSV file\n");
        return 1;
    }
    printf("Read %zu records from yok_atlas.csv\n", n);

    // 2) B+ Ağacını oluştur ve tüm kayıtları ardışık ekleme ile yerleştir
    BPNode *root = NULL;
    for (size_t i = 0; i < n; ++i) {
        insert_sequential(&root, &recs[i]);
    }
    printf("Tree built. Root num_keys = %d\n",
           root ? root->num_keys : 0);

    // 3) Örnek aramalar yap
    //    a) İlk kaydın bölümünden 1. üniversiteyi getir
    const char *dept0 = recs[0].department;
    UniListNode *u1 = search(root, dept0, 1);
    if (u1) {
        printf("1st university in \"%s\": %s (%.2f)\n",
               dept0, u1->university, u1->score);
    } else {
        printf("Department \"%s\" not found or has no entries\n", dept0);
    }

    //    b) Aynı bölümde 2. üniversiteyi getir (varsa)
    UniListNode *u2 = search(root, dept0, 2);
    if (u2) {
        printf("2nd university in \"%s\": %s (%.2f)\n",
               dept0, u2->university, u2->score);
    } else {
        printf("Department \"%s\" has fewer than 2 universities\n", dept0);
    }

    // 4) Kaynakları temizle
    free_records(recs, n);
    // TODO: B+ ağacını da serbest bırakmak için bir destroy function ekleyebilirsin

    return 0;
}
