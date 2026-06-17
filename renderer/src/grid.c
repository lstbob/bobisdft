#include "grid.h"
#include "tui.h"
#include "date_utils.h"
#include "storage.h"

#include <stdio.h>
#include <string.h>

const char *VIEW_NAMES[VIEW_COUNT] = {
    "Daily",
    "Weekly",
    "Monthly"
};

void grid_clear(void) {
    printf("\033[2J\033[H");
    fflush(stdout);
}

static void print_trunc(const char *s, int max_width) {
    int len = (int)strlen(s);
    if (len <= max_width) {
        printf("%s", s);
        for (int i = len; i < max_width; i++) putchar(' ');
    } else {
        printf("%.*s", max_width - 1, s);
        putchar('>');
    }
}

int days_in_month(int year, int month) {
    int dim[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)) {
        return 29;
    }
    return dim[month - 1];
}

static const char *DOW_SHORT[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

void grid_draw_day(const DiaryEntry *entry, TermSize term) {
    char date_str[MAX_DATE_STR_LEN];
    format_date("long", entry->date, date_str, sizeof(date_str));
    (void)term;

    printf("\n  " ANSI_BOLD ANSI_CYAN "%s" ANSI_RESET "\n\n", date_str);
    printf("  " ANSI_BOLD ANSI_YELLOW "%s" ANSI_RESET "\n", entry->title);

    if (entry->content && entry->content_len > 0) {
        printf("\n%s\n", entry->content);
    } else {
        printf("\n" ANSI_DIM "(empty entry)" ANSI_RESET "\n");
    }
}

int col_width(TermSize term) {
    int w = (term.cols - 8) / 7;
    if (w < 8) w = 8;
    return w;
}

void grid_draw_week(DiaryEntry entries[7], int count, TermSize term) {
    int cw = col_width(term);

    printf("\n  " ANSI_BOLD ANSI_CYAN "Weekly View" ANSI_RESET "\n\n");

    for (int d = 0; d < count; d++) {
        if (d > 0) printf(" ");
        char buf[32];
        int dow = day_of_week(entries[d].date);
        snprintf(buf, sizeof(buf), "%s %02d", DOW_SHORT[dow], entries[d].date.day);
        printf(ANSI_BOLD ANSI_YELLOW "%-*s" ANSI_RESET, cw, buf);
    }
    printf("\n");

    for (int d = 0; d < count; d++) {
        if (d > 0) printf(" ");
        char cell[128];
        if (entries[d].title[0]) {
            snprintf(cell, sizeof(cell), "%s", entries[d].title);
            printf(ANSI_GREEN);
            print_trunc(cell, cw);
            printf(ANSI_RESET);
        } else {
            cell[0] = '\0';
            printf(ANSI_DIM);
            print_trunc(cell, cw);
            printf(ANSI_RESET);
        }
    }
    printf("\n");
}

void grid_draw_month(int year, int month, const Date *entries, int entry_count, TermSize term) {
    (void)term;
    const char *mn = month_name(month);
    char month_str[64];
    if (mn && mn[0]) {
        char cap[32];
        cap[0] = (char)(mn[0] - 32);
        size_t i = 1;
        while (mn[i] && i < sizeof(cap) - 1) {
            cap[i] = mn[i];
            i++;
        }
        cap[i] = '\0';
        snprintf(month_str, sizeof(month_str), "%s %d", cap, year);
    } else {
        snprintf(month_str, sizeof(month_str), "%d", year);
    }

    printf("\n  " ANSI_BOLD ANSI_CYAN "%s" ANSI_RESET "\n\n", month_str);

    for (int i = 0; i < 7; i++) {
        if (i > 0) printf(" ");
        printf(ANSI_BOLD ANSI_YELLOW "%3s" ANSI_RESET, DOW_SHORT[i]);
    }
    printf("\n");

    Date first = {year, month, 1};
    int start_dow = day_of_week(first);
    int dim = days_in_month(year, month);

    for (int i = 0; i < start_dow; i++) {
        if (i > 0) printf(" ");
        printf("    ");
    }

    for (int day = 1; day <= dim; day++) {
        if ((day + start_dow - 1) % 7 == 0 && day > 1) {
            printf("\n");
        } else if (day > 1) {
            printf(" ");
        }

        int has_entry = 0;
        for (int i = 0; i < entry_count; i++) {
            if (entries[i].year == year && entries[i].month == month && entries[i].day == day) {
                has_entry = 1;
                break;
            }
        }

        if (has_entry) {
            printf(ANSI_REVERSE ANSI_GREEN "%3d" ANSI_RESET, day);
        } else {
            printf("%3d", day);
        }
    }
    printf("\n");
}
