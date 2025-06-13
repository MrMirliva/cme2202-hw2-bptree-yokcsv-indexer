// external_sort.c

#define _XOPEN_SOURCE 700
#include "external_sort.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>    // unlink()

#define RUN_SIZE 1000   // tek bir run’da saklanacak maksimum kayıt sayısı
#define LINE_BUF 4096

// Satırları department ↑, score ↓ bazında karşılaştırır
static int record_cmp_line(const char *a, const char *b) {
    char ta[LINE_BUF], tb[LINE_BUF];
    strcpy(ta, a);
    strcpy(tb, b);

    // CSV: id,university,department,score
    char *f;
    f = strtok(ta, ",");          // id
    f = strtok(NULL, ",");        // university
    f = strtok(NULL, ",");        // department
    char *dept_a = f;
    f = strtok(NULL, ",");        // score
    double score_a = f ? atof(f) : 0.0;

    f = strtok(tb, ",");
    f = strtok(NULL, ",");
    f = strtok(NULL, ",");
    char *dept_b = f;
    f = strtok(NULL, ",");
    double score_b = f ? atof(f) : 0.0;

    int c = strcmp(dept_a, dept_b);
    if (c) return c;
    // Aynı departman: skora göre ters
    if (score_a < score_b) return  1;
    if (score_a > score_b) return -1;
    return 0;
}

// qsort için wrapper
static int qsort_cmp(const void *pa, const void *pb) {
    const char *a = *(const char * const *)pa;
    const char *b = *(const char * const *)pb;
    return record_cmp_line(a, b);
}

// CSV’den RUN_SIZE’lık parçalar okuyup her birini sort edip runXX.csv’e yazar
char **generate_runs(const char *csv_path, int *out_run_count) {
    FILE *in = fopen(csv_path, "r");
    if (!in) { perror(csv_path); exit(1); }

    // başlık satırını atla
    char line[LINE_BUF];
    if (!fgets(line, LINE_BUF, in)) { fclose(in); return NULL; }

    char **buffer   = malloc(RUN_SIZE * sizeof(char*));
    char **run_files= NULL;
    int bufcount = 0, runcount = 0;

    while (fgets(line, LINE_BUF, in)) {
        buffer[bufcount++] = strdup(line);
        if (bufcount == RUN_SIZE) {
            qsort(buffer, bufcount, sizeof(char*), qsort_cmp);
            char name[32];
            snprintf(name, sizeof(name), "run%02d.csv", runcount);
            FILE *out = fopen(name, "w");
            for (int i = 0; i < bufcount; i++) {
                fputs(buffer[i], out);
                free(buffer[i]);
            }
            fclose(out);

            run_files = realloc(run_files, (runcount+1)*sizeof(char*));
            run_files[runcount++] = strdup(name);
            bufcount = 0;
        }
    }
    // Kalan son run
    if (bufcount > 0) {
        qsort(buffer, bufcount, sizeof(char*), qsort_cmp);
        char name[32];
        snprintf(name, sizeof(name), "run%02d.csv", runcount);
        FILE *out = fopen(name, "w");
        for (int i = 0; i < bufcount; i++) {
            fputs(buffer[i], out);
            free(buffer[i]);
        }
        fclose(out);

        run_files = realloc(run_files, (runcount+1)*sizeof(char*));
        run_files[runcount++] = strdup(name);
    }

    free(buffer);
    fclose(in);
    *out_run_count = runcount;
    return run_files;
}

// Oluşturulan run’ları k-way merge ile tek dosyada toplar
char *merge_runs(char **runs, int run_count) {
    FILE **fps    = malloc(run_count * sizeof(FILE*));
    char **bufline= malloc(run_count * sizeof(char*));
    for (int i = 0; i < run_count; i++) {
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
        for (int i = 0; i < run_count; i++) {
            if (!bufline[i]) continue;
            if (best < 0 || record_cmp_line(bufline[i], bufline[best]) < 0)
                best = i;
        }
        if (best < 0) break;  // bitti
        fputs(bufline[best], out);
        if (!fgets(bufline[best], LINE_BUF, fps[best])) {
            free(bufline[best]);
            bufline[best] = NULL;
        }
    }

    fclose(out);
    for (int i = 0; i < run_count; i++) fclose(fps[i]);
    free(fps);
    free(bufline);

    return strdup(out_name);
}

// Dış sıralama: önce run’ları üret, sonra merge et, temp dosyaları sil
char *external_sort(const char *csv_path) {
    int run_count;
    char **runs = generate_runs(csv_path, &run_count);
    char *sorted = merge_runs(runs, run_count);
    for (int i = 0; i < run_count; i++) {
        unlink(runs[i]);
        free(runs[i]);
    }
    free(runs);
    return sorted;
}
