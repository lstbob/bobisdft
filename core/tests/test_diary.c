#include "diary.h"
#include "date_utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int tests_run = 0;
static int tests_failed = 0;

#define TEST(name) do { printf("  %s ... ", name); tests_run++; } while(0)
#define PASS() printf("ok\n")
#define FAIL(msg) do { printf("FAIL (%s)\n", msg); tests_failed++; } while(0)
#define ASSERT(cond, msg) do { if (!(cond)) { FAIL(msg); return; } } while(0)
#define ASSERT_EQ(a, b, msg) do { if ((a) != (b)) { FAIL(msg); return; } } while(0)

static void test_entry_clear(void) {
    TEST("entry_clear");
    DiaryEntry e;
    e.date.year = 2026;
    e.title[0] = 'x';
    e.content = (char *)0x1234;
    e.content_len = 99;
    entry_clear(&e);
    ASSERT_EQ(e.date.year, 0, "year zeroed");
    ASSERT_EQ(e.title[0], '\0', "title cleared");
    ASSERT(e.content == NULL, "content null");
    ASSERT_EQ(e.content_len, (size_t)0, "len zero");
    PASS();
}

static void test_entry_free(void) {
    TEST("entry_free null content");
    DiaryEntry e;
    entry_clear(&e);
    entry_free(&e);
    ASSERT(e.content == NULL, "content null after free");
    PASS();
}

static void test_entry_free_allocated(void) {
    TEST("entry_free allocated");
    DiaryEntry e;
    entry_clear(&e);
    e.content = strdup("hello");
    e.content_len = 5;
    entry_free(&e);
    ASSERT(e.content == NULL, "content freed");
    ASSERT_EQ(e.content_len, (size_t)0, "len zero");
    PASS();
}

int main(void) {
    printf("diary tests:\n");
    test_entry_clear();
    test_entry_free();
    test_entry_free_allocated();

    printf("\n%d tests, %d failed\n", tests_run, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
