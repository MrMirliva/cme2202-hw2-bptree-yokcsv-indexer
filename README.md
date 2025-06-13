# cme2202-hw2-bptree-yokcsv-indexer

# C11 standardı, XOPEN_SOURCE strdup için, tüm modüller
gcc -std=c11 -D_XOPEN_SOURCE=700 \
    -Wall -Wextra -g \
    record.c bptree.c external_sort.c Bulk_Load.c main.c \
    -o app

# 1) Sıralı ekleme (sequential insertion)
./app 1

# 2) Toplu yükleme (bulk loading) → önce external sort, sonra ağaç inşası
./app 2
