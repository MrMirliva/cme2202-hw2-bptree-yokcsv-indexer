# B+ Tree Indexer (Assignment 2)

This project indexes a CSV dataset (`yok_atlas.csv`) by department using a **B+ tree**, and compares the performance of two tree-building methods: **Sequential Insertion** and **Bottom-up Bulk Load**.

## Features

- **CSV Reading**: Parses records via `read_records()` in `record.c`.
- **External Sort**: Generates `sorted.csv` using replacement selection and multi-way merge in `external_sort.c`.
- **Sequential Insertion**: Inserts records one by one into the B+ tree with `insert_sequential()`.
- **Bottom-up Bulk Load**: Builds the tree in two phases—leaf creation then internal nodes—via `bulk_load()`.
- **Performance Metrics**: Reports tree height, split count, memory usage, and build time for both methods.
- **Search Capability**: Queries the k-th university for a given department using `search()`.

## Requirements

- C compiler with C99 support
- POSIX timers (`-D_POSIX_C_SOURCE=199309L`)
- `librt` for `clock_gettime`

## Build Instructions

Compile all source files and link `librt`:

```bash
gcc -std=c99 -D_POSIX_C_SOURCE=199309L \
    main.c record.c bptree.c external_sort.c -o bptree_app -lrt
```

## Usage

```bash
./bptree_app <csv_file>
# Example:
./bptree_app yok_atlas.csv
```

The program executes the following steps:
1. Perform external sort to create `sorted.csv`.
2. Load records with `read_records()`.
3. Prompt user to choose tree-building method:
   - `1`: Sequential Insertion
   - `2`: Bottom-up Bulk Load
4. Build the B+ tree and display metrics (build time, height, splits, memory).
5. Enter search mode: prompt for department name and rank k, then display the result.

## Examples

**Sequential Insertion**

```
Select B+ Tree load method:
  1) Sequential Insertion
  2) Bottom-up Bulk Load
Enter choice (1 or 2): 1

[Sequential Insertion]
Build Time   : 0.001525 s
Tree Height  : 9
Split Count  : 700
Memory Usage : 450988 bytes

Enter department name (exact): Fizik
Enter rank k (1-based): 4

4th university in "Fizik": ORTA DOGU TEKNIK UNIVERSITESI (495.68)
Search Time  : 0.000002885 s
```

**Bottom-up Bulk Load**

```
Select B+ Tree load method:
  1) Sequential Insertion
  2) Bottom-up Bulk Load
Enter choice (1 or 2): 2

[Bottom-up Bulk Load]
Build Time   : 0.000750 s
Tree Height  : 5
Split Count  : 0
Memory Usage : 384218 bytes

Enter department name (exact): Fizik
Enter rank k (1-based): 4

4th university in "Fizik": ORTA DOGU TEKNIK UNIVERSITESI (495.68)
Search Time  : 0.000002355 s
```

## Implementation Details

- **B+ Tree Order** (`BP_DEGREE`): Set to `4` (max 3 keys per node).
- **Memory Measurement**: Tracks bytes allocated through `malloc` for nodes and lists.
- **Split Counting**: Global `g_split_count` increments on every `split_node()`.
- **Search**: Locates the leaf via `find_leaf()` and scans the `UniListNode` chain.

## Results and Comparison

Performance charts and a detailed report are available in the `reports/` folder under `performance_comparison.png`. Included plots:

- **Build Time vs. Method**
- **Tree Height vs. Method**
- **Split Count vs. Method**
- **Memory Usage vs. Method**

In the report, **bulk-load** generally yields fewer splits and lower memory usage, while **sequential insertion** performs similarly on small datasets.

---

**Authors:** Abdullah Gündüz, Eren Burak Uzun

**Submission:** Package all `.c` files, this `README.md`, and any generated charts into a ZIP archive.
