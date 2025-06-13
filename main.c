// main.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "record.h"
#include "bptree.h"
#include "external_sort.h"
#include "Bulk_Load.h"    // bulk_load() burada tanımlı

extern size_t g_split_count;            // split_node her çağrıldığında++
extern size_t g_memory_usage_bytes;     // malloc/realloc izi
int tree_height(BPNode *root);           // ağacın yüksekliğini hesaplar

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr,
            "Kullanım: %s <1|2>\n"
            "  1 - Sequential Insertion\n"
            "  2 - Bulk Loading (external merge sort)\n",
            argv[0]);
        return EXIT_FAILURE;
    }
    int mode = atoi(argv[1]);
    if (mode != 1 && mode != 2) {
        fprintf(stderr, "Geçersiz mod: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    const char *csv_file = "yok_atlas.csv";
    char *to_read = (char*)csv_file;
    if (mode == 2) {
        printf("1) Bulk loading: external sort başlıyor...\n");
        to_read = external_sort(csv_file);
        printf("   Sıralı dosya: %s\n", to_read);
    } else {
        printf("1) Sequential insertion: doğrudan CSV okunuyor...\n");
    }

    size_t n;
    Record *recs = read_records(to_read, &n);
    if (!recs) {
        fprintf(stderr, "Kayıt okunamadı: %s\n", to_read);
        return EXIT_FAILURE;
    }
    printf("   Toplam %zu kayıt okundu.\n", n);

    BPNode *root = NULL;
    g_split_count = 0;
    g_memory_usage_bytes = 0;
    clock_t t0 = clock();

    if (mode == 1) {
        printf("2) Sequential insertion modunda ağaç inşa ediliyor...\n");
        for (size_t i = 0; i < n; i++)
            insert_sequential(&root, &recs[i]);
    } else {
        printf("2) Bulk loading modunda ağaç inşa ediliyor...\n");
        bulk_load(&root, recs, n);
    }

    double build_time = (double)(clock() - t0) / CLOCKS_PER_SEC;
    printf("   Ağaç inşası tamamlandı.\n");

    printf("\n=== Metrikler ===\n");
    printf("Split sayısı        : %zu\n", g_split_count);
    printf("Tahmini bellek      : %.2f MB\n", g_memory_usage_bytes/(1024.0*1024.0));
    printf("Ağaç yüksekliği     : %d\n", tree_height(root));
    printf("İnşa süresi         : %.4f sn\n", build_time);
    printf("===================\n\n");

    char dept[256];
    int rank;
    printf("Arama için bölüm adı girin: ");
    fgets(dept, sizeof(dept), stdin);
    dept[strcspn(dept, "\r\n")] = '\0';

    printf("Üniversite sırasını girin (1-tabanlı): ");
    if (scanf("%d", &rank) != 1) rank = 1;

    clock_t t1 = clock();
    UniListNode *res = search(root, dept, rank);
    double seek_time = (double)(clock() - t1) / CLOCKS_PER_SEC;

    if (res)
        printf("\nArama sonucu: %s (%.2f)\n", res->university, res->score);
    else
        printf("\nBölüm veya sıra bulunamadı.\n");
    printf("Arama süresi: %.6f sn\n", seek_time);

    destroy_tree(root);
    free_records(recs, n);
    if (mode == 2) free(to_read);

    return EXIT_SUCCESS;
}
