// record.h

#ifndef RECORD_H
#define RECORD_H

#include <stddef.h>

/**
 * Tek bir CSV kaydını temsil eder.
 * - id: Bölüm sırası (UUID olarak)
 * - university: Üniversite adı
 * - department: Bölüm adı
 * - score: Yerleştirme puanı
 */
typedef struct {
    char *id;           // e.g. "1"
    char *university;   // e.g. "YEDITEPE UNIVERSITESI"
    char *department;   // e.g. "Gastronomi ve Mutfak Sanatlari"
    double score;       // e.g. 580.0
} Record;

/**
 * CSV dosyasını satır satır okuyup dinamik bir Record dizisi oluşturur.
 * @param filename   CSV dosya yolu
 * @param out_count  Kayıt sayısını alacak pointer
 * @return Oluşturulan Record dizisi veya NULL (hata durumunda).
 *   Dönen diziyi ve içindeki stringleri free_records ile temizleyin.
 */
Record *read_records(const char *filename, size_t *out_count);

/**
 * read_records ile oluşturulan Record dizisini ve içindeki stringleri serbest bırakır.
 * @param arr    Record dizisi
 * @param count  Dizideki kayıt sayısı
 */
void free_records(Record *arr, size_t count);

#endif // RECORD_H
