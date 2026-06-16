#include "date_utils.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

static const char *MONTH_NAMES[] = {
    "january", "february", "march", "april", "may", "june",
    "july", "august", "september", "october", "november", "december"
};

static const char *MONTH_ABBR[] = {
    "jan", "feb", "mar", "apr", "may", "jun",
    "jul", "aug", "sep", "oct", "nov", "dec"
};

static const char *DOW_NAMES[] = {
    "sunday", "monday", "tuesday", "wednesday",
    "thursday", "friday", "saturday"
};

static const char *DOW_ABBR[] = {
    "sun", "mon", "tue", "wed", "thu", "fri", "sat"
};

bool date_valid(Date d) {
    if (d.year < 2020 || d.year > 2099) return false;
    if (d.month < 1 || d.month > 12) return false;
    if (d.day < 1 || d.day > 31) return false;

    int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if ((d.year % 4 == 0 && d.year % 100 != 0) || (d.year % 400 == 0)) {
        days_in_month[1] = 29;
    }

    return d.day <= days_in_month[d.month - 1];
}

int date_cmp(Date a, Date b) {
    if (a.year != b.year) return a.year - b.year;
    if (a.month != b.month) return a.month - b.month;
    return a.day - b.day;
}

static int month_from_name(const char *s) {
    for (int i = 0; i < 12; i++) {
        if (strcmp(s, MONTH_NAMES[i]) == 0 || strcmp(s, MONTH_ABBR[i]) == 0) {
            return i + 1;
        }
    }
    return 0;
}

static int dow_from_name(const char *s) {
    for (int i = 0; i < 7; i++) {
        if (strcmp(s, DOW_NAMES[i]) == 0 || strcmp(s, DOW_ABBR[i]) == 0) {
            return i;
        }
    }
    return -1;
}

static void str_lower(char *dst, const char *src, size_t max_len) {
    size_t i = 0;
    while (src[i] && i < max_len - 1) {
        dst[i] = (char)tolower((unsigned char)src[i]);
        i++;
    }
    dst[i] = '\0';
}

static int parse_iso_date(const char *s, Date *out) {
    int y, m, d;
    if (sscanf(s, "%d-%d-%d", &y, &m, &d) == 3) {
        out->year = y;
        out->month = m;
        out->day = d;
        if (date_valid(*out)) return 1;
    }
    return 0;
}

static int parse_us_date(const char *s, Date *out) {
    char month_str[32];
    int d, y;
    if (sscanf(s, "%31s %d, %d", month_str, &d, &y) == 3) {
        char lower[32];
        str_lower(lower, month_str, sizeof(lower));
        int m = month_from_name(lower);
        if (m) {
            out->year = y;
            out->month = m;
            out->day = d;
            if (date_valid(*out)) return 1;
        }
    }
    return 0;
}

static int parse_eu_date(const char *s, Date *out) {
    char month_str[32];
    int d, y;
    if (sscanf(s, "%d %31s %d", &d, month_str, &y) == 3) {
        char lower[32];
        str_lower(lower, month_str, sizeof(lower));
        int m = month_from_name(lower);
        if (m) {
            out->year = y;
            out->month = m;
            out->day = d;
            if (date_valid(*out)) return 1;
        }
    }
    return 0;
}

static int parse_relative_day(const char *s, Date *out) {
    if (strcmp(s, "today") == 0) {
        *out = date_today();
        return 1;
    }
    if (strcmp(s, "tomorrow") == 0) {
        *out = date_tomorrow();
        return 1;
    }
    return 0;
}

static int parse_next_dow(const char *s, Date *out) {
    char dow_str[32];
    if (sscanf(s, "next %31s", dow_str) == 1) {
        char lower[32];
        str_lower(lower, dow_str, sizeof(lower));
        int target_dow = dow_from_name(lower);
        if (target_dow >= 0) {
            Date today = date_today();
            int current_dow = day_of_week(today);
            int days_ahead = target_dow - current_dow;
            if (days_ahead <= 0) days_ahead += 7;
            *out = add_days(today, days_ahead);
            return 1;
        }
    }
    return 0;
}

bool parse_date(const char *input, Date *out) {
    if (!input || !out) return false;

    char lower[128];
    str_lower(lower, input, sizeof(lower));

    if (parse_relative_day(lower, out)) return true;
    if (parse_next_dow(lower, out)) return true;
    if (parse_iso_date(lower, out)) return true;
    if (parse_us_date(lower, out)) return true;
    if (parse_eu_date(lower, out)) return true;

    return false;
}

static void cap_first(char *dst, const char *src, size_t max_len) {
    if (src[0]) {
        dst[0] = (char)toupper((unsigned char)src[0]);
        size_t i = 1;
        while (src[i] && i < max_len - 1) {
            dst[i] = src[i];
            i++;
        }
        dst[i] = '\0';
    } else {
        dst[0] = '\0';
    }
}

int format_date(const char *fmt, Date d, char *out, size_t out_len) {
    if (!fmt || strcmp(fmt, "iso") == 0) {
        return snprintf(out, out_len, "%04d-%02d-%02d", d.year, d.month, d.day);
    }
    if (strcmp(fmt, "long") == 0) {
        char month_cap[32];
        cap_first(month_cap, MONTH_NAMES[d.month - 1], sizeof(month_cap));
        return snprintf(out, out_len, "%s %d, %d",
                        month_cap, d.day, d.year);
    }
    return snprintf(out, out_len, "%04d-%02d-%02d", d.year, d.month, d.day);
}

Date date_today(void) {
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    Date d = {
        .year = local->tm_year + 1900,
        .month = local->tm_mon + 1,
        .day = local->tm_mday
    };
    return d;
}

Date date_tomorrow(void) {
    return add_days(date_today(), 1);
}

Date add_days(Date d, int days) {
    struct tm tm = {0};
    tm.tm_year = d.year - 1900;
    tm.tm_mon = d.month - 1;
    tm.tm_mday = d.day;
    tm.tm_hour = 12;

    mktime(&tm);
    tm.tm_mday += days;
    mktime(&tm);

    Date result = {
        .year = tm.tm_year + 1900,
        .month = tm.tm_mon + 1,
        .day = tm.tm_mday
    };
    return result;
}

int day_of_week(Date d) {
    struct tm tm = {0};
    tm.tm_year = d.year - 1900;
    tm.tm_mon = d.month - 1;
    tm.tm_mday = d.day;
    tm.tm_hour = 12;

    mktime(&tm);
    return tm.tm_wday;
}

Date week_start(Date d) {
    int dow = day_of_week(d);
    return add_days(d, -dow);
}

const char *month_name(int month) {
    if (month < 1 || month > 12) return NULL;
    return MONTH_NAMES[month - 1];
}
