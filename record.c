// record.c

#include "record.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 16
#define LINE_BUF_SIZE   1024

// Güvenli strdup: malloc başarısızsa NULL döner
static char *strdup_safe(const char *s) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char *dup = malloc(len);
    if (dup) {
        memcpy(dup, s, len);
    }
    return dup;
}

Record *read_records(const char *filename, size_t *out_count) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror(filename);
        return NULL;
    }

    // Başlık satırını atla
    char line[LINE_BUF_SIZE];
    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        return NULL;
    }

    size_t capacity = INITIAL_CAPACITY;
    size_t count    = 0;
    Record *arr     = malloc(capacity * sizeof *arr);
    if (!arr) {
        perror("malloc");
        fclose(fp);
        return NULL;
    }

    while (fgets(line, sizeof(line), fp)) {
        // Satır sonu karakterlerini temizle
        line[strcspn(line, "\r\n")] = '\0';

        // 4 alan bekliyoruz: id,university,department,score
        char *fields[4];
        size_t i = 0;
        char *tok = strtok(line, ",");
        while (tok && i < 4) {
            fields[i++] = tok;
            tok = strtok(NULL, ",");
        }
        if (i != 4) {
            fprintf(stderr, "Malformed line (4 fields expected): %s\n", line);
            continue;
        }

        // Gerekirse dizi boyutunu iki katına çıkar
        if (count == capacity) {
            size_t new_cap = capacity * 2;
            Record *tmp = realloc(arr, new_cap * sizeof *arr);
            if (!tmp) {
                perror("realloc");
                free_records(arr, count);
                fclose(fp);
                return NULL;
            }
            arr = tmp;
            capacity = new_cap;
        }

        // Kaydı doldur
        arr[count].id         = strdup_safe(fields[0]);
        arr[count].university = strdup_safe(fields[1]);
        arr[count].department = strdup_safe(fields[2]);
        arr[count].score      = strtod(fields[3], NULL);
        ++count;
    }

    fclose(fp);

    // Fazladan ayrılan belleği geri kes (opsiyonel)
    if (count < capacity) {
        Record *tmp = realloc(arr, count * sizeof *arr);
        if (tmp) {
            arr = tmp;
        }
    }

    *out_count = count;
    return arr;
}

void free_records(Record *arr, size_t count) {
    if (!arr) return;
    for (size_t i = 0; i < count; ++i) {
        free(arr[i].id);
        free(arr[i].university);
        free(arr[i].department);
    }
    free(arr);
}
