// external_sort.h

#ifndef EXTERNAL_SORT_H
#define EXTERNAL_SORT_H

// generate sorted runs; returns malloc’la ayrılmış string dizisi
// out_run_count’a run sayısını yaz
char **generate_runs(const char *csv_path, int *out_run_count);

// merge sorted runs into one file, cleans up run files;
// returns malloc’la ayrılmış sorted file path
char *merge_runs(char **runs, int run_count);

// full external sort: combine above two
char *external_sort(const char *csv_path);

#endif // EXTERNAL_SORT_H
