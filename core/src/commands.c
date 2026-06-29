#include "commands.h"
#include "diary.h"
#include "date_utils.h"
#include "storage.h"
#include "input.h"
#include "sanitize.h"
#include "editor.h"
#include "tui.h"
#include "grid.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void print_entry(const DiaryEntry *entry, const char *label) {
    char date_str[MAX_DATE_STR_LEN];
    format_date("long", entry->date, date_str, sizeof(date_str));
    printf("\n" ANSI_BOLD "%s:" ANSI_RESET " %s\n", label, date_str);
    printf("  " ANSI_BOLD ANSI_YELLOW "Title:" ANSI_RESET " %s\n", entry->title);
    printf("  " ANSI_BOLD ANSI_CYAN "Content:" ANSI_RESET "\n");
    if (entry->content && entry->content_len > 0) {
        printf("%s\n", entry->content);
    } else {
        printf("  " ANSI_DIM "(empty)" ANSI_RESET "\n");
    }
}

static bool entry_day_interactive(Date date, bool *skipped) {
    char date_str[MAX_DATE_STR_LEN];
    format_date("long", date, date_str, sizeof(date_str));
    printf("\n--- Entry for %s ---\n", date_str);

    if (entry_exists(date)) {
        DiaryEntry existing;
        entry_clear(&existing);
        load_entry(date, &existing);
        print_entry(&existing, "Existing entry");
        entry_free(&existing);

        if (!confirm("Overwrite existing entry")) {
            if (skipped) *skipped = true;
            return false;
        }
    }

    printf("Title: ");
    fflush(stdout);
    char title[MAX_TITLE_LEN];
    if (!read_line(title, sizeof(title)) || title[0] == '\0') {
        printf("Cancelled.\n");
        return false;
    }

    printf("Opening editor to write content...\n");
    char *content = NULL;
    size_t content_len = 0;

    if (!open_editor("", 0, &content, &content_len)) {
        fprintf(stderr, "error: editor failed\n");
        return false;
    }

    DiaryEntry entry;
    entry_clear(&entry);
    entry.date = date;
    snprintf(entry.title, MAX_TITLE_LEN, "%s", title);
    entry.content = content;
    entry.content_len = content_len;

    if (!save_entry(&entry)) {
        fprintf(stderr, "error: could not save entry for %s\n", date_str);
        entry_free(&entry);
        return false;
    }

    printf("Saved entry for %s\n", date_str);
    entry_free(&entry);
    if (skipped) *skipped = false;
    return true;
}

static bool edit_entry_interactive(Date date) {
    char date_str[MAX_DATE_STR_LEN];
    format_date("long", date, date_str, sizeof(date_str));

    if (!entry_exists(date)) {
        printf("No entry for %s\n", date_str);
        return false;
    }

    DiaryEntry existing;
    entry_clear(&existing);
    if (!load_entry(date, &existing)) {
        printf("Could not load entry for %s\n", date_str);
        return false;
    }

    printf("Editing entry for %s\n", date_str);
    printf("Title: %s\n", existing.title);
    printf("Opening editor...\n");

    char *new_content = NULL;
    size_t new_len = 0;
    if (!open_editor(existing.content, existing.content_len,
                     &new_content, &new_len)) {
        fprintf(stderr, "error: editor failed\n");
        entry_free(&existing);
        return false;
    }

    existing.content_len = new_len;
    free(existing.content);
    existing.content = new_content;

    if (!save_entry(&existing)) {
        fprintf(stderr, "error: could not save entry\n");
        entry_free(&existing);
        return false;
    }

    printf("Saved entry for %s\n", date_str);
    entry_free(&existing);
    return true;
}

static Date prompt_date_default(Date def) {
    char ds[MAX_DATE_STR_LEN];
    format_date("long", def, ds, sizeof(ds));
    printf("Date for entry (Enter for %s, or type a date): ", ds);
    fflush(stdout);

    char input[128];
    if (!read_line(input, sizeof(input)) || input[0] == '\0') {
        return def;
    }

    Date d;
    if (parse_date(input, &d)) return d;

    printf("Could not parse date. Using %s.\n", ds);
    return def;
}

static Date prompt_date(void) {
    return prompt_date_default(date_tomorrow());
}

int cmd_ne(int argc, char *argv[]) {
    bool weekly = (argc >= 3 && strcmp(argv[2], "-w") == 0);

    if (weekly) {
        Date start = date_today();
        printf("Planning weekly entries starting %04d-%02d-%02d\n",
               start.year, start.month, start.day);

        int planned = 0;
        int skipped = 0;
        for (int i = 0; i < 7; i++) {
            Date d = add_days(start, i);
            bool was_skipped = false;
            if (!entry_day_interactive(d, &was_skipped)) {
                if (was_skipped) skipped++;
            } else {
                planned++;
            }
        }
        printf("\nWeekly entries complete: %d days written, %d days kept existing.\n",
               planned, skipped);
    } else {
        Date target = prompt_date();
        entry_day_interactive(target, NULL);
    }

    return 0;
}

static void load_entries_for_month(int year, int month, DiaryEntry *out, int *count) {
    Date start = {year, month, 1};
    int dim = 31;
    if (month == 2) {
        dim = ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) ? 29 : 28;
    } else if (month == 4 || month == 6 || month == 9 || month == 11) {
        dim = 30;
    }
    Date end = {year, month, dim};

    Date dates[31];
    int found = list_entries_in_range(start, end, dates, 31);
    *count = 0;
    for (int i = 0; i < found && *count < 31; i++) {
        if (load_entry(dates[i], &out[*count])) {
            (*count)++;
        }
    }
}

int cmd_se(int argc, char *argv[]) {
    Date cursor = date_today();
    ViewMode view = VIEW_DAILY;

    if (argc >= 4 && strcmp(argv[2], "-d") == 0) {
        Date d;
        if (parse_date(argv[3], &d)) {
            cursor = d;
        }
    }

    raw_mode_enable();
    atexit(raw_mode_disable);

    int running = 1;
    while (running) {
        TermSize term = get_term_size();
        grid_clear();

        switch (view) {
            case VIEW_DAILY: {
                DiaryEntry entry;
                entry_clear(&entry);
                if (load_entry(cursor, &entry)) {
                    grid_draw_day(&entry, term);
                    entry_free(&entry);
                } else {
                    char ds[MAX_DATE_STR_LEN];
                    format_date("long", cursor, ds, sizeof(ds));
                    printf("\n  " ANSI_BOLD ANSI_CYAN "%s" ANSI_RESET "\n\n", ds);
                    printf("  " ANSI_DIM "(no entry for this day)" ANSI_RESET "\n");
                }
                break;
            }
            case VIEW_WEEKLY: {
                Date start = week_start(cursor);
                DiaryEntry entries[7];
                int count = 0;
                for (int i = 0; i < 7; i++) {
                    Date d = add_days(start, i);
                    entry_clear(&entries[i]);
                    entries[i].date = d;
                    if (load_entry(d, &entries[i])) {
                        count++;
                    }
                }
                if (count == 0) {
                    printf("\n  " ANSI_BOLD ANSI_CYAN "Weekly View" ANSI_RESET "\n\n");
                    printf("  " ANSI_DIM "(no entries this week)" ANSI_RESET "\n");
                } else {
                    grid_draw_week(entries, 7, term);
                }
                for (int i = 0; i < 7; i++) entry_free(&entries[i]);
                break;
            }
            case VIEW_MONTHLY: {
                DiaryEntry entries[31];
                int count;
                load_entries_for_month(cursor.year, cursor.month, entries, &count);
                Date plan_dates[31];
                for (int i = 0; i < count; i++) {
                    plan_dates[i] = entries[i].date;
                }
                grid_draw_month(cursor.year, cursor.month, plan_dates, count, term);
                for (int i = 0; i < count; i++) entry_free(&entries[i]);
                break;
            }
            default:
                break;
        }

        printf("\n"
               "  " ANSI_DIM "[" ANSI_RESET ANSI_BOLD ANSI_GREEN "i" ANSI_RESET ANSI_DIM "]" ANSI_RESET "%s"
               "  " ANSI_DIM "[" ANSI_RESET ANSI_BOLD ANSI_GREEN "n" ANSI_RESET ANSI_DIM "]next" ANSI_RESET
               "  " ANSI_DIM "[" ANSI_RESET ANSI_BOLD ANSI_GREEN "N" ANSI_RESET ANSI_DIM "]prev" ANSI_RESET
               "  " ANSI_DIM "[" ANSI_RESET ANSI_BOLD ANSI_GREEN "e" ANSI_RESET ANSI_DIM "]entry" ANSI_RESET
               "  " ANSI_DIM "[" ANSI_RESET ANSI_BOLD ANSI_RED "q" ANSI_RESET ANSI_DIM "]quit" ANSI_RESET,
               view == VIEW_DAILY ? " weekly" : (view == VIEW_WEEKLY ? " monthly" : " daily"));
        printf("\n");
        fflush(stdout);

        int key = read_key();
        switch (key) {
            case 'q':
            case KEY_ESC:
                running = 0;
                break;
            case 'i':
                view = (ViewMode)((view + 1) % VIEW_COUNT);
                break;
            case 'n':
                switch (view) {
                    case VIEW_DAILY:   cursor = add_days(cursor, 1);       break;
                    case VIEW_WEEKLY:  cursor = add_days(cursor, 7);       break;
                    case VIEW_MONTHLY:
                        if (cursor.month == 12) { cursor.year++; cursor.month = 1; }
                        else { cursor.month++; }
                        break;
                    default: break;
                }
                break;
            case 'N':
                switch (view) {
                    case VIEW_DAILY:   cursor = add_days(cursor, -1);      break;
                    case VIEW_WEEKLY:  cursor = add_days(cursor, -7);      break;
                    case VIEW_MONTHLY:
                        if (cursor.month == 1) { cursor.year--; cursor.month = 12; }
                        else { cursor.month--; }
                        break;
                    default: break;
                }
                break;
            case 'e': {
                raw_mode_disable();
                printf("\n");
                Date d = prompt_date_default(cursor);
                if (entry_exists(d)) edit_entry_interactive(d);
                else                 entry_day_interactive(d, NULL);
                raw_mode_enable();
                break;
            }
            case KEY_RIGHT:
                if (view == VIEW_DAILY) cursor = add_days(cursor, 1);
                else if (view == VIEW_WEEKLY) cursor = add_days(cursor, 7);
                break;
            case KEY_LEFT:
                if (view == VIEW_DAILY) cursor = add_days(cursor, -1);
                else if (view == VIEW_WEEKLY) cursor = add_days(cursor, -7);
                break;
            default:
                break;
        }
    }

    raw_mode_disable();
    printf("\n");
    return 0;
}

int cmd_ee(int argc, char *argv[]) {
    Date target;

    if (argc >= 4 && strcmp(argv[2], "-d") == 0) {
        Date d;
        if (parse_date(argv[3], &d)) {
            target = d;
        } else {
            fprintf(stderr, "Could not parse date.\n");
            return 1;
        }
    } else {
        target = prompt_date();
    }

    if (!entry_exists(target)) {
        char ds[MAX_DATE_STR_LEN];
        format_date("long", target, ds, sizeof(ds));
        printf("No entry for %s\n", ds);
        return 1;
    }

    edit_entry_interactive(target);
    return 0;
}
