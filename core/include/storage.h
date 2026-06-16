#ifndef STORAGE_H
#define STORAGE_H

#include "diary.h"
#include <stdbool.h>
#include <stddef.h>

#define DATA_DIR "data"

bool storage_init(void);
bool save_entry(const DiaryEntry *entry);
bool load_entry(Date date, DiaryEntry *out);
bool entry_exists(Date date);
int list_entries_in_range(Date start, Date end, Date *out, size_t max_count);

#endif
