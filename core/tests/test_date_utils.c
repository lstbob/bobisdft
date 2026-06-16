#include "date_utils.h"
#include <stdio.h>
#include <string.h>

static int tests_run = 0;
static int tests_failed = 0;

#define TEST(name) do { printf("  %s ... ", name); tests_run++; } while(0)
#define PASS() printf("ok\n")
#define FAIL(msg) do { printf("FAIL (%s)\n", msg); tests_failed++; } while(0)
#define ASSERT(cond, msg) do { if (!(cond)) { FAIL(msg); return; } } while(0)
#define ASSERT_EQ(a, b, msg) do { if ((a) != (b)) { FAIL(msg); return; } } while(0)

static void test_date_valid(void) {
    TEST("valid date");
    Date d = {2026, 6, 17};
    ASSERT(date_valid(d), "should be valid");
    PASS();
}

static void test_date_invalid(void) {
    TEST("invalid month");
    Date d = {2026, 13, 1};
    ASSERT(!date_valid(d), "month 13 invalid");
    PASS();
}

static void test_leap_year(void) {
    TEST("leap year Feb 29");
    Date d = {2024, 2, 29};
    ASSERT(date_valid(d), "2024-02-29 valid");
    PASS();
}

static void test_date_cmp(void) {
    TEST("date_cmp");
    Date a = {2026, 6, 17};
    Date b = {2026, 6, 17};
    Date c = {2026, 6, 18};
    ASSERT_EQ(date_cmp(a, b), 0, "equal");
    ASSERT(date_cmp(a, c) < 0, "before");
    ASSERT(date_cmp(c, a) > 0, "after");
    PASS();
}

static void test_parse_iso(void) {
    TEST("parse ISO");
    Date d;
    ASSERT(parse_date("2026-06-17", &d), "ok");
    ASSERT_EQ(d.year, 2026, "year");
    ASSERT_EQ(d.month, 6, "month");
    ASSERT_EQ(d.day, 17, "day");
    PASS();
}

static void test_parse_us(void) {
    TEST("parse US long");
    Date d;
    ASSERT(parse_date("June 17, 2026", &d), "ok");
    ASSERT_EQ(d.month, 6, "month");
    PASS();
}

static void test_parse_eu(void) {
    TEST("parse EU");
    Date d;
    ASSERT(parse_date("17 Jun 2026", &d), "ok");
    ASSERT_EQ(d.day, 17, "day");
    PASS();
}

static void test_parse_today(void) {
    TEST("parse today");
    Date d;
    ASSERT(parse_date("today", &d), "ok");
    Date today = date_today();
    ASSERT_EQ(d.year, today.year, "year");
    PASS();
}

static void test_format_iso(void) {
    TEST("format ISO");
    Date d = {2026, 6, 17};
    char buf[64];
    format_date("iso", d, buf, sizeof(buf));
    ASSERT(strcmp(buf, "2026-06-17") == 0, "ISO");
    PASS();
}

static void test_format_long(void) {
    TEST("format long");
    Date d = {2026, 6, 17};
    char buf[64];
    format_date("long", d, buf, sizeof(buf));
    ASSERT(strcmp(buf, "June 17, 2026") == 0, "long");
    PASS();
}

static void test_add_days(void) {
    TEST("add_days");
    Date d = {2026, 12, 31};
    Date r = add_days(d, 1);
    ASSERT_EQ(r.year, 2027, "year");
    ASSERT_EQ(r.month, 1, "month");
    ASSERT_EQ(r.day, 1, "day");
    PASS();
}

static void test_day_of_week(void) {
    TEST("day_of_week");
    Date d = {2026, 6, 17};
    ASSERT_EQ(day_of_week(d), 3, "Wednesday");
    PASS();
}

static void test_week_start(void) {
    TEST("week_start");
    Date d = {2026, 6, 17};
    Date ws = week_start(d);
    ASSERT_EQ(ws.day, 14, "Sunday June 14");
    PASS();
}

int main(void) {
    printf("date_utils tests:\n");
    test_date_valid();
    test_date_invalid();
    test_leap_year();
    test_date_cmp();
    test_parse_iso();
    test_parse_us();
    test_parse_eu();
    test_parse_today();
    test_format_iso();
    test_format_long();
    test_add_days();
    test_day_of_week();
    test_week_start();

    printf("\n%d tests, %d failed\n", tests_run, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
