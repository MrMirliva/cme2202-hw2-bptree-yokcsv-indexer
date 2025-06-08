// record.c

#include "record.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 16
#define LINE_BUF_SIZE   1024

static char *strdup_safe(const char *s) {
    char *dup = malloc(strlen(s) + 1);
    if (dup) {
        strcpy(dup, s);
    }
    return dup;
}

/**
 * Reads all records from a CSV file (expects header on first line).
 * @param filename   path to CSV file
 * @param out_count  pointer to size_t that will receive number of records
 * @return pointer to dynamically-allocated array of Record; NULL on error.
 *   Caller is responsible for freeing both the array and all Record fields.
 */
Record *read_records(const char *filename, size_t *out_count) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Failed to open CSV");
        return NULL;
    }

    char line[LINE_BUF_SIZE];
    // Skip header line
    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        return NULL;
    }

    size_t capacity = INITIAL_CAPACITY;
    size_t count = 0;
    Record *arr = malloc(capacity * sizeof(Record));
    if (!arr) {
        perror("malloc");
        fclose(fp);
        return NULL;
    }

    while (fgets(line, sizeof(line), fp)) {
        // Remove trailing newline
        line[strcspn(line, "\r\n")] = '\0';

        // Tokenize on commas
        char *tok = strtok(line, ",");
        if (!tok) continue;
        char *fields[4];
        int i = 0;
        for (; tok && i < 4; ++i) {
            fields[i] = tok;
            tok = strtok(NULL, ",");
        }
        if (i != 4) {
            fprintf(stderr, "Malformed line (expected 4 fields): %s\n", line);
            continue;
        }

        // Expand array if needed
        if (count == capacity) {
            capacity *= 2;
            Record *tmp = realloc(arr, capacity * sizeof(Record));
            if (!tmp) {
                perror("realloc");
                break;
            }
            arr = tmp;
        }

        // Populate Record
        arr[count].id         = strdup_safe(fields[0]);
        arr[count].university = strdup_safe(fields[1]);
        arr[count].department = strdup_safe(fields[2]);
        arr[count].score      = strtod(fields[3], NULL);
        ++count;
    }

    fclose(fp);
    *out_count = count;
    return arr;
}

/**
 * Frees the array of Record structs and all internal strings.
 * @param arr    Pointer to the Record array
 * @param count  Number of records in the array
 */
void free_records(Record *arr, size_t count) {
    if (!arr) return;
    for (size_t i = 0; i < count; ++i) {
        free(arr[i].id);
        free(arr[i].university);
        free(arr[i].department);
    }
    free(arr);
}
