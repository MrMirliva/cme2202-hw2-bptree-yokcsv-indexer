// main.c

#define _POSIX_C_SOURCE 199309L
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "record.h"
#include "bptree.h"
#include "external_sort.h"

int main(void) {
    const char *in_csv = "yok_atlas.csv";

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

    // common timers
    struct timespec t0, t1;

    // ---- Sequential Insert ----
    BPNode *root_seq = NULL;
    g_split_count = 0;
    g_memory_usage_bytes = 0;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (size_t i = 0; i < n; i++) {
        insert_sequential(&root_seq, &recs[i]);
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double seq_time = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec)/1e9;
    int    seq_height = tree_height(root_seq);
    size_t seq_splits = g_split_count;
    size_t seq_mem    = g_memory_usage_bytes;

    // ---- Bulk Load ----
    BPNode *root_bulk = NULL;
    g_split_count = 0;
    g_memory_usage_bytes = 0;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    bulk_load(&root_bulk, recs, n);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double bulk_time = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec)/1e9;
    int    bulk_height = tree_height(root_bulk);
    size_t bulk_splits = g_split_count;
    size_t bulk_mem    = g_memory_usage_bytes;

    // 3) Karşılaştırmalı sonuçları yazdır
    printf("Method       Time(s)    Height   Splits    Memory(bytes)\n");
    printf("Sequential   %8.6f     %4d     %4zu     %12zu\n",
           seq_time, seq_height, seq_splits, seq_mem);
    printf("Bulk         %8.6f     %4d     %4zu     %12zu\n\n",
           bulk_time, bulk_height, bulk_splits, bulk_mem);

    // 4) Örnek arama (bulk ağacı üzerinden)
    const char *dept = "Gastronomi ve Mutfak Sanatlari";
    printf("Search results for \"%s\":\n", dept);
    for (int k = 1; k <= 2; ++k) {
        UniListNode *u = search(root_bulk, dept, k);
        if (u) {
            printf("  %d%s: %s (%.2f)\n",
                   k, (k==1?"st":"nd"), u->university, u->score);
        } else {
            printf("  %d%s: not found\n", k, (k==1?"st":"nd"));
        }
    }
    printf("\n");

    // 5) Ortalama seek time ölçümü
    // 5a) Farklı departmanları topla
    size_t D = 0;
    char **depts = malloc(n * sizeof *depts);
    for (size_t i = 0; i < n; ) {
        depts[D++] = recs[i].department;
        size_t j = i+1;
        while (j < n && strcmp(recs[j].department, recs[i].department) == 0) j++;
        i = j;
    }

    // 5b) Her departman için k=1..min(5,ulCount) arama yap, süreyi topla
    size_t total_searches = 0;
    long   total_ns = 0;
    struct timespec s0, s1;
    for (size_t di = 0; di < D; ++di) {
        // liste uzunluğunu bul
        UniListNode *u = search(root_bulk, depts[di], 1);
        int ulCount = 0;
        for (UniListNode *t = u; t; t = t->next) ulCount++;
        int maxk = ulCount < 5 ? ulCount : 5;
        for (int k = 1; k <= maxk; ++k) {
            clock_gettime(CLOCK_MONOTONIC, &s0);
            search(root_bulk, depts[di], k);
            clock_gettime(CLOCK_MONOTONIC, &s1);
            long diff = (s1.tv_sec - s0.tv_sec)*1000000000L
                      + (s1.tv_nsec - s0.tv_nsec);
            total_ns += diff;
            total_searches++;
        }
    }
    double avg_seek = total_ns / (double)total_searches / 1e9;
    printf("Average seek time: %.9f s over %zu searches\n\n",
           avg_seek, total_searches);
    free(depts);

    // 6) Temizlik
    destroy_tree(root_seq);
    destroy_tree(root_bulk);
    free_records(recs, n);
    free(sorted_csv);

    return EXIT_SUCCESS;
}
