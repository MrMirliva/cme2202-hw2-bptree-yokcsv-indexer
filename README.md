# B+ Tree Assignment 2

## Overview
This project indexes university placement scores from a CSV file by department name using a B+ tree and compares two loading strategies:

1. **Sequential Insertion**: Records are inserted into the B+ tree one by one, performing splits as necessary to maintain balance.
2. **Bottom-up Bulk Loading**: Records are first sorted via external sort, then leaf nodes are created in bulk, and internal nodes are built top-down. This minimizes the number of splits.

Metrics such as build time, split count, memory usage, tree height, and search time are measured and reported.

## File Structure

- **main.c**: The program's entry point. Takes the CSV path and loading option, runs external sort, reads records, builds the B+ tree, reports metrics, and offers an interactive search prompt.
- **bptree.h / bptree.c**: B+ tree implementation. The order is defined by the constant `BP_DEGREE` (default is 4). Functions include:
  - `create_node`, `insert_sequential`, `bulk_load`, `search`, `tree_height`, `destroy_tree`.
- **external_sort.h / external_sort.c**: Generates run files via replacement-selection and merges them to produce `sorted.csv`.
- **record.h / record.c**: Contains `read_records` to parse CSV lines into an array of `Record` structs and `free_records` to clean up.
- **Assignment2.pdf**: The assignment specification.
- **README_EN.md**: This documentation in English.

## Features

- **Two Loading Strategies**:
  - Sequential Insertion (step-by-step insertion)
  - Bottom-up Bulk Loading (batch leaf and internal node construction)
- **External Merge Sort**: Replacement-selection heap for disk-based sorting of large files.
- **Dynamic Metrics**: Tracks build time, split count, memory usage, tree height, and search time.
- **Interactive Search**: Prompts for department name and rank to return the k-th university.

## Compilation

```bash
gcc -std=c11 -D_POSIX_C_SOURCE=199309L -Wall -Wextra -pedantic -g *.c -o bptree
```

## Usage

```bash
./bptree [csv_file]
```

- `csv_file`: The CSV file to process (default is `yok_atlas.csv`)

Program flow:
1. Run external sort → generates `sorted.csv`.
2. Read records → print total record count.
3. Select loading method: `1` for Sequential, `2` for Bulk.
4. Build the B+ tree → print metrics.
5. Enter department name and k → display search results.

## Example

```
./bptree yok_atlas.csv
Select B+ Tree load method:
  1) Sequential Insertion
  2) Bottom-up Bulk Load
Enter choice (1 or 2): 2

[Bottom-up Bulk Load]
Build Time   : 0.001110 s
Tree Height  : 5
Split Count  : 0
Memory Usage : 392444 bytes

Enter department name (exact): Physics
Enter rank k (1-based): 4
4th university in "Physics": MIDDLE EAST TECHNICAL UNIVERSITY (495.68)
Search Time  : 0.00000527 s
```

## Notes

- The tree order can be changed by updating the `#define BP_DEGREE` in `bptree.h`.
- Temporary run files created during external sort are deleted after completion.
- All dynamic memory usage is tracked via the `g_memory_usage_bytes` counter.

## Authors

- Abdullah Gündüz  
- Eren Burak Uzun  
