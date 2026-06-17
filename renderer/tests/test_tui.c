#include "tui.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int tests_run = 0;
static int tests_failed = 0;

#define TEST(name) do { printf("  %s ... ", name); tests_run++; } while(0)
#define PASS() printf("ok\n")
#define FAIL(msg) do { printf("FAIL (%s)\n", msg); tests_failed++; } while(0)
#define ASSERT(cond, msg) do { if (!(cond)) { FAIL(msg); return; } } while(0)
#define ASSERT_EQ(a, b, msg) do { if ((a) != (b)) { FAIL(msg); return; } } while(0)

static void test_term_size_defaults(void) {
    TEST("get_term_size defaults");
    TermSize ts = get_term_size();
    ASSERT(ts.rows >= 10, "rows >= 10");
    ASSERT(ts.cols >= 10, "cols >= 10");
    PASS();
}

static void test_raw_mode_safe(void) {
    TEST("raw_mode enable/disable does not crash");
    raw_mode_enable();
    raw_mode_disable();
    raw_mode_disable();
    PASS();
}

static void test_key_none(void) {
    TEST("read_key returns NONE with no input");
    int key = read_key();
    ASSERT_EQ(key, KEY_NONE, "should be KEY_NONE");
    PASS();
}

static void test_key_mapping(void) {
    TEST("key mapping constants unique");
    ASSERT(KEY_NONE != KEY_UP, "NONE != UP");
    ASSERT(KEY_UP != KEY_DOWN, "UP != DOWN");
    ASSERT(KEY_DOWN != KEY_LEFT, "DOWN != LEFT");
    ASSERT(KEY_LEFT != KEY_RIGHT, "LEFT != RIGHT");
    ASSERT(KEY_RIGHT != KEY_ESC, "RIGHT != ESC");
    ASSERT(KEY_ESC != KEY_TAB, "ESC != TAB");
    PASS();
}

int main(void) {
    printf("tui tests:\n");
    test_term_size_defaults();
    test_raw_mode_safe();
    test_key_none();
    test_key_mapping();

    printf("\n%d tests, %d failed\n", tests_run, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
