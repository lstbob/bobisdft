#ifndef GRID_H
#define GRID_H

#include "tui.h"
#include "diary.h"

typedef enum {
    VIEW_DAILY,
    VIEW_WEEKLY,
    VIEW_MONTHLY,
    VIEW_COUNT
} ViewMode;

extern const char *VIEW_NAMES[VIEW_COUNT];

void grid_clear(void);
void grid_draw_day(const DiaryEntry *entry, TermSize term);
void grid_draw_week(DiaryEntry entries[7], int count, TermSize term);
void grid_draw_month(int year, int month, const Date *entries, int entry_count, TermSize term);

#endif
