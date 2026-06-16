#ifndef DIARY_H
#define DIARY_H

#include "date_utils.h"
#include <stdbool.h>
#include <stddef.h>

#define MAX_TITLE_LEN 128

typedef struct {
    Date date;
    char title[MAX_TITLE_LEN];
    char *content;
    size_t content_len;
} DiaryEntry;

void entry_clear(DiaryEntry *e);
void entry_free(DiaryEntry *e);

#endif
