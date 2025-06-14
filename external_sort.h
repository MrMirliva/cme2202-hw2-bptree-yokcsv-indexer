#ifndef EXTERNAL_SORT_H
#define EXTERNAL_SORT_H

#include <stddef.h>

// Generates sorted runs from the CSV at 'csv_path'.
// Returns an array of malloc’ed C-strings, one per run file path.
// Writes the number of runs into *out_run_count.
// Caller must free each string in the returned array and then free the array itself.
char **generate_runs(const char *csv_path, size_t *out_run_count);

// Merges 'run_count' sorted run files (paths in 'runs') into a single sorted CSV.
// Cleans up intermediate run files.
// Returns a malloc’ed C-string containing the path to the final sorted file.
// Caller must free the returned string.
char *merge_runs(char **runs, size_t run_count);

// Performs full external sort:
// 1) generate_runs()
// 2) merge_runs()
// 3) cleanup of run files
//
// Returns a malloc’ed C-string with the path to the sorted CSV.
// Caller must free the returned string.
char *external_sort(const char *csv_path);

#endif // EXTERNAL_SORT_H
