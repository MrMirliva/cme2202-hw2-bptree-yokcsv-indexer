// record.h

#ifndef RECORD_H
#define RECORD_H

#include <stddef.h>

typedef struct {
    char *id;           // e.g. "1"
    char *university;   // e.g. "YEDITEPE UNIVERSITESI"
    char *department;   // e.g. "Gastronomi ve Mutfak Sanatlari"
    double score;       // e.g. 580.0
} Record;

/**
 * Reads all records from a CSV file (expects header on first line).
 * @param filename   path to CSV file
 * @param out_count  pointer to size_t that will receive number of records
 * @return pointer to dynamically-allocated array of Record; NULL on error.
 *   Caller is responsible for freeing both the array and all Record fields.
 */
Record *read_records(const char *filename, size_t *out_count);

/**
 * Frees the array of Record structs and all internal strings.
 * @param arr    Pointer to the Record array
 * @param count  Number of records in the array
 */
void free_records(Record *arr, size_t count);

#endif // RECORD_H
