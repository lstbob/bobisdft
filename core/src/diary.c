#include "diary.h"
#include <stdlib.h>
#include <string.h>

void entry_clear(DiaryEntry *e) {
    if (!e) return;
    e->date.year = 0;
    e->date.month = 0;
    e->date.day = 0;
    e->title[0] = '\0';
    e->content = NULL;
    e->content_len = 0;
}

void entry_free(DiaryEntry *e) {
    if (!e) return;
    free(e->content);
    e->content = NULL;
    e->content_len = 0;
}
