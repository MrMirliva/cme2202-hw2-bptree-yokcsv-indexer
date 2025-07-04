#define _POSIX_C_SOURCE 199309L
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "record.h"
#include "bptree.h"
#include "external_sort.h"

int main(int argc, char *argv[]) {
    // 3 argüman: <csv_file> <1|2> <order>
    if (argc != 4) {
        fprintf(stderr,
            "Usage: %s <csv_file> <1|2> <order>\n"
            "  <csv_file> : Input CSV file\n"
            "  1          : Sequential Insertion\n"
            "  2          : Bottom-up Bulk Load\n"
            "  <order>    : B+ tree order (>=3)\n",
            argv[0]);
        return EXIT_FAILURE;
    }

    const char *in_csv = argv[1];
    int choice         = atoi(argv[2]);
    bp_degree          = atoi(argv[3]);

    if (choice != 1 && choice != 2) {
        fprintf(stderr, "Error: Invalid load method: %d\n", choice);
        return EXIT_FAILURE;
    }
    if (bp_degree < 3) {
        fprintf(stderr, "Error: order must be at least 3 (got %d)\n", bp_degree);
        return EXIT_FAILURE;
    }

    // 1) CSV’i dışsal sıralayıp yeni dosya adını al
    char *sorted_csv = external_sort(in_csv);
    if (!sorted_csv) {
        fprintf(stderr, "Error: external_sort failed on %s\n", in_csv);
        return EXIT_FAILURE;
    }
    printf("External sort complete. Sorted file: %s\n", sorted_csv);

    // 2) Sıralı kayıtları oku
    size_t n;
    Record *recs = read_records(sorted_csv, &n);
    if (!recs) {
        fprintf(stderr, "Error: failed to read records from %s\n", sorted_csv);
        free(sorted_csv);
        return EXIT_FAILURE;
    }
    printf("Read %zu records from %s\n\n", n, sorted_csv);

    // 3) Ağacı kur ve ölç
    struct timespec t0, t1;
    BPNode *root = NULL;
    double elapsed;
    int height;
    size_t splits, mem;

    if (choice == 1) {
        // Sequential Insertion
        g_split_count = 0;
        g_memory_usage_bytes = 0;
        clock_gettime(CLOCK_MONOTONIC, &t0);
        for (size_t i = 0; i < n; i++) {
            insert_sequential(&root, &recs[i]);
        }
        clock_gettime(CLOCK_MONOTONIC, &t1);
        printf("\n[Sequential Insertion]\n");
    } else {
        // Bottom-up Bulk Load
        g_split_count = 0;
        g_memory_usage_bytes = 0;
        clock_gettime(CLOCK_MONOTONIC, &t0);
        bulk_load(&root, recs, n);
        clock_gettime(CLOCK_MONOTONIC, &t1);
        printf("\n[Bottom-up Bulk Load]\n");
    }

    elapsed = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec)/1e9;
    height  = tree_height(root);
    splits  = g_split_count;
    mem     = g_memory_usage_bytes;

    // 4) Metrikleri yazdır
    printf("Build Time   : %.6f s\n", elapsed);
    printf("Tree Height  : %d\n",      height);
    printf("Split Count  : %zu\n",     splits);
    printf("Memory Usage : %zu bytes\n\n", mem);

    // 5) Etkileşimli arama
    char dept_buf[256];
    int k;
    printf("Enter department name (exact): ");
    if (!fgets(dept_buf, sizeof dept_buf, stdin)) {
        fprintf(stderr, "Error reading department\n");
        destroy_tree(root);
        free_records(recs, n);
        free(sorted_csv);
        return EXIT_FAILURE;
    }
    dept_buf[strcspn(dept_buf, "\r\n")] = '\0';

    printf("Enter rank k (1-based): ");
    if (scanf("%d", &k) != 1 || k < 1) {
        fprintf(stderr, "Invalid rank\n");
        destroy_tree(root);
        free_records(recs, n);
        free(sorted_csv);
        return EXIT_FAILURE;
    }

    clock_gettime(CLOCK_MONOTONIC, &t0);
    UniListNode *u = search(root, dept_buf, k);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double search_time = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec)/1e9;

    if (u) {
        printf("\n%d%s university in \"%s\": %s (%.2f)\n",
               k,
               (k == 1 ? "st" : (k == 2 ? "nd" : "th")),
               dept_buf, u->university, u->score);
    } else {
        printf("\nNot found: %d%s university in \"%s\"\n",
               k,
               (k == 1 ? "st" : (k == 2 ? "nd" : "th")),
               dept_buf);
    }
    printf("Search Time  : %.9f s\n", search_time);

    // 6) Temizlik
    destroy_tree(root);
    free_records(recs, n);
    free(sorted_csv);

    return EXIT_SUCCESS;
}
