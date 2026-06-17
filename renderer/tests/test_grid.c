#include "grid.h"
#include "tui.h"
#include "diary.h"
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

static void test_col_width_normal(void) {
    TEST("col_width normal");
    TermSize ts = {24, 120};
    int cw = col_width(ts);
    ASSERT_EQ(cw, 16, "120 cols -> 16 per cell");
    PASS();
}

static void test_col_width_narrow(void) {
    TEST("col_width narrow");
    TermSize ts = {24, 60};
    int cw = col_width(ts);
    ASSERT_EQ(cw, 8, "60 cols -> min 8");
    PASS();
}

static void test_col_width_very_narrow(void) {
    TEST("col_width very narrow");
    TermSize ts = {24, 30};
    int cw = col_width(ts);
    ASSERT_EQ(cw, 8, "30 cols -> min 8");
    PASS();
}

static void test_col_width_wide(void) {
    TEST("col_width wide");
    TermSize ts = {24, 200};
    int cw = col_width(ts);
    ASSERT_EQ(cw, 27, "200 cols -> 27 per cell");
    PASS();
}

static void test_days_in_month_normal(void) {
    TEST("days_in_month normal");
    ASSERT_EQ(days_in_month(2026, 1), 31, "Jan");
    ASSERT_EQ(days_in_month(2026, 2), 28, "Feb");
    ASSERT_EQ(days_in_month(2026, 4), 30, "Apr");
    PASS();
}

static void test_days_in_month_leap(void) {
    TEST("days_in_month leap year");
    ASSERT_EQ(days_in_month(2024, 2), 29, "2024 Feb");
    ASSERT_EQ(days_in_month(2000, 2), 29, "2000 Feb");
    ASSERT_EQ(days_in_month(2100, 2), 28, "2100 Feb (not leap)");
    PASS();
}

int main(void) {
    printf("grid tests:\n");
    test_col_width_normal();
    test_col_width_narrow();
    test_col_width_very_narrow();
    test_col_width_wide();
    test_days_in_month_normal();
    test_days_in_month_leap();

    printf("\n%d tests, %d failed\n", tests_run, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
