// external_sort.c

#include "external_sort.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RUN_SIZE 1000  // bir run’da saklanacak maksimum kayıt sayısı

// Karşılaştırma: department ↑, score ↓
static int record_cmp(const void *a, const void *b) {
    const char *la = *(const char **)a;
    const char *lb = *(const char **)b;
    // CSV satırındaki 3. alan department, 4. alan score
    // basit parse: departmanı al
    const char *da = strchr(la, ',');
    da = da ? strchr(da+1, ',') + 1 : la;
    const char *db = strchr(lb, ',');
    db = db ? strchr(db+1, ',') + 1 : lb;
    int dc = strcspn(da, ",");
    int dd = strcspn(db, ",");
    int cmp = strncmp(da, db, dc < dd ? dc : dd);
    if (cmp != 0) return cmp;
    if (dc != dd) return dc < dd ? -1 : 1;
    // skor karşılaştırması
    double sa = atof(da + dc + 1);
    double sb = atof(db + dd + 1);
    return (sa < sb) - (sa > sb);
}

char **generate_runs(const char *csv_path, int *out_run_count) {
    FILE *in = fopen(csv_path, "r");
    if (!in) return NULL;

    char line[4096];
    fgets(line, sizeof(line), in);  // başlık

    char **buffer = malloc(RUN_SIZE * sizeof(char *));
    int bufcount = 0, runcount = 0;
    char **run_files = NULL;

    while (fgets(line, sizeof(line), in)) {
        buffer[bufcount++] = strdup(line);
        if (bufcount == RUN_SIZE) {
            qsort(buffer, bufcount, sizeof(char *), record_cmp);
            char name[64];
            snprintf(name, sizeof(name), "run%02d.csv", runcount);
            FILE *out = fopen(name, "w");
            for (int i = 0; i < bufcount; i++) {
                fputs(buffer[i], out);
                free(buffer[i]);
            }
            fclose(out);
            run_files = realloc(run_files, (runcount+1)*sizeof(char *));
            run_files[runcount++] = strdup(name);
            bufcount = 0;
        }
    }
    if (bufcount > 0) {
        qsort(buffer, bufcount, sizeof(char *), record_cmp);
        char name[64];
        snprintf(name, sizeof(name), "run%02d.csv", runcount);
        FILE *out = fopen(name, "w");
        for (int i = 0; i < bufcount; i++) {
            fputs(buffer[i], out);
            free(buffer[i]);
        }
        fclose(out);
        run_files = realloc(run_files, (runcount+1)*sizeof(char *));
        run_files[runcount++] = strdup(name);
    }

    free(buffer);
    fclose(in);
    *out_run_count = runcount;
    return run_files;
}

char *merge_runs(char **runs, int run_count) {
    // Tüm run dosyalarından satırları oku
    char **all = NULL;
    size_t total = 0, cap = 0;
    char line[4096];
    for (int i = 0; i < run_count; i++) {
        FILE *in = fopen(runs[i], "r");
        if (!in) continue;
        while (fgets(line, sizeof(line), in)) {
            if (total == cap) {
                cap = cap ? cap * 2 : 1024;
                all = realloc(all, cap * sizeof(char *));
            }
            all[total++] = strdup(line);
        }
        fclose(in);
        // geçici file’ı sil ve adı free et
        remove(runs[i]);
        free(runs[i]);
    }
    free(runs);

    // tamamını sıralayıp tek dosyaya yaz
    qsort(all, total, sizeof(char *), record_cmp);
    char *out_name = strdup("sorted.csv");
    FILE *out = fopen(out_name, "w");
    for (size_t i = 0; i < total; i++) {
        fputs(all[i], out);
        free(all[i]);
    }
    fclose(out);
    free(all);
    return out_name;
}

char *external_sort(const char *csv_path) {
    int run_count = 0;
    char **runs = generate_runs(csv_path, &run_count);
    if (!runs) return NULL;
    return merge_runs(runs, run_count);
}
