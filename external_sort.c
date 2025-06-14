#define _XOPEN_SOURCE 700
#include "external_sort.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>    // unlink()
#include <stddef.h>    // size_t

#define RUN_SIZE   1000    // Bellekte tutabileceğimiz maksimum run elemanı
#define LINE_BUF   4096

// Kayıt karşılaştırması: dept ↑, score ↓
static int record_cmp_line(const char *a, const char *b) {
    char ta[LINE_BUF], tb[LINE_BUF];
    strcpy(ta, a);
    strcpy(tb, b);

    char *f = strtok(ta, ",");        // id
    f = strtok(NULL, ",");            // university
    f = strtok(NULL, ",");            // department
    char *dept_a = f;
    f = strtok(NULL, ",");            // score
    double score_a = f ? atof(f) : 0.0;

    f = strtok(tb, ",");
    f = strtok(NULL, ",");
    f = strtok(NULL, ",");
    char *dept_b = f;
    f = strtok(NULL, ",");
    double score_b = f ? atof(f) : 0.0;

    int c = strcmp(dept_a, dept_b);
    if (c) return c;
    if (score_a < score_b) return  1;
    if (score_a > score_b) return -1;
    return 0;
}

// Replacement-selection düğümü
typedef struct {
    char *line;      // tam CSV satırı
    int   run;       // bu kaydın run numarası
} RSNode;

// RSNode karşılaştırması
static int rsnode_cmp(const RSNode *a, const RSNode *b) {
    if (a->run != b->run) return a->run - b->run;
    return record_cmp_line(a->line, b->line);
}

// Min-heap heapify
static void heapify(RSNode *heap, size_t size, size_t i) {
    size_t smallest = i;
    size_t l = 2*i + 1, r = 2*i + 2;
    if (l < size && rsnode_cmp(&heap[l], &heap[smallest]) < 0)
        smallest = l;
    if (r < size && rsnode_cmp(&heap[r], &heap[smallest]) < 0)
        smallest = r;
    if (smallest != i) {
        RSNode tmp = heap[i];
        heap[i] = heap[smallest];
        heap[smallest] = tmp;
        heapify(heap, size, smallest);
    }
}

// Heap inşa et
static void build_heap(RSNode *heap, size_t size) {
    for (int i = (int)size/2 - 1; i >= 0; --i)
        heapify(heap, size, i);
}

char **generate_runs(const char *csv_path, size_t *out_run_count) {
    FILE *in = fopen(csv_path, "r");
    if (!in) { perror(csv_path); return NULL; }

    // başlık satırını atla
    char header[LINE_BUF];
    if (!fgets(header, LINE_BUF, in)) {
        fclose(in);
        return NULL;
    }

    // heap için yer ayır
    RSNode *heap = malloc(RUN_SIZE * sizeof *heap);
    if (!heap) { perror("malloc"); fclose(in); return NULL; }
    size_t heap_size = 0;
    char buf[LINE_BUF];

    // İlk RUN_SIZE satırı oku
    while (heap_size < RUN_SIZE && fgets(buf, LINE_BUF, in)) {
        heap[heap_size].line = strdup(buf);
        heap[heap_size].run  = 0;
        heap_size++;
    }
    if (heap_size == 0) {
        free(heap);
        fclose(in);
        *out_run_count = 0;
        return NULL;
    }
    build_heap(heap, heap_size);

    char **run_files = NULL;
    size_t run_count = 0;

    // İlk run dosyasını aç
    char name[32];
    snprintf(name, sizeof(name), "run%02zu.csv", run_count);
    FILE *out = fopen(name, "w");
    if (!out) { perror(name); free(heap); fclose(in); return NULL; }
    run_files = realloc(run_files, (run_count+1)*sizeof *run_files);
    run_files[run_count++] = strdup(name);

    // Asıl replacement-selection döngüsü
    while (heap_size > 0) {
        // En küçük eleman
        RSNode top = heap[0];

        // Eğer run numarası değiştiyse yeni run dosyası aç
        if (top.run != (int)run_count-1) {
            fclose(out);
            snprintf(name, sizeof(name), "run%02zu.csv", run_count);
            out = fopen(name, "w");
            if (!out) { perror(name); break; }
            run_files = realloc(run_files, (run_count+1)*sizeof *run_files);
            run_files[run_count++] = strdup(name);
        }

        // Satırı yaz
        fputs(top.line, out);

        // Yeni input satırı oku
        if (fgets(buf, LINE_BUF, in)) {
            RSNode newn;
            newn.line = strdup(buf);
            // run numarasını hesapla **top.line** hala geçerli
            if (!newn.line) { perror("strdup"); exit(1); }
            newn.run  = (record_cmp_line(newn.line, top.line) >= 0
                         ? top.run : top.run + 1);
            // Artık eski top.line'ı serbest bırak
            free(top.line);
            // heap’in kökünü yeni node ile değiştir
            heap[0] = newn;
        } else {
            // Input bitti: heap’in tepesini sondan al ve boyutu küçült
            free(top.line);
            heap[0] = heap[heap_size-1];
            heap_size--;
        }
        // Heap’i düzelt
        heapify(heap, heap_size, 0);
    }

    fclose(out);
    free(heap);
    fclose(in);

    *out_run_count = run_count;
    return run_files;
}

// Mevcut merge_runs ve external_sort:
char *merge_runs(char **runs, size_t run_count) {
    FILE **fps    = malloc(run_count * sizeof *fps);
    char **bufline= malloc(run_count * sizeof *bufline);

    for (size_t i = 0; i < run_count; i++) {
        fps[i] = fopen(runs[i], "r");
        if (!fps[i]) { perror(runs[i]); exit(1); }
        bufline[i] = malloc(LINE_BUF);
        if (!fgets(bufline[i], LINE_BUF, fps[i])) {
            free(bufline[i]);
            bufline[i] = NULL;
        }
    }

    const char *out_name = "sorted.csv";
    FILE *out = fopen(out_name, "w");
    if (!out) { perror(out_name); exit(1); }

    while (1) {
        int best = -1;
        for (size_t i = 0; i < run_count; i++) {
            if (!bufline[i]) continue;
            if (best < 0 || record_cmp_line(bufline[i], bufline[best]) < 0)
                best = (int)i;
        }
        if (best < 0) break;
        fputs(bufline[best], out);
        if (!fgets(bufline[best], LINE_BUF, fps[best])) {
            free(bufline[best]);
            bufline[best] = NULL;
        }
    }

    fclose(out);
    for (size_t i = 0; i < run_count; i++) fclose(fps[i]);
    free(fps);
    free(bufline);

    return strdup(out_name);
}

char *external_sort(const char *csv_path) {
    size_t run_count;
    char **runs = generate_runs(csv_path, &run_count);
    if (!runs) return NULL;
    char *sorted = merge_runs(runs, run_count);
    for (size_t i = 0; i < run_count; i++) {
        unlink(runs[i]);
        free(runs[i]);
    }
    free(runs);
    return sorted;
}
