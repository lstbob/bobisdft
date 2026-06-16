#include "sanitize.h"
#include <stdio.h>
#include <string.h>

static int tests_run = 0;
static int tests_failed = 0;

#define TEST(name) do { printf("  %s ... ", name); tests_run++; } while(0)
#define PASS() printf("ok\n")
#define FAIL(msg) do { printf("FAIL (%s)\n", msg); tests_failed++; } while(0)
#define ASSERT(cond, msg) do { if (!(cond)) { FAIL(msg); return; } } while(0)
#define ASSERT_EQ(a, b, msg) do { if ((a) != (b)) { FAIL(msg); return; } } while(0)

static void test_normal(void) {
    TEST("normal unchanged");
    char buf[] = "Hello";
    sanitize_string(buf, sizeof(buf));
    ASSERT(strcmp(buf, "Hello") == 0, "");
    PASS();
}

static void test_control_chars(void) {
    TEST("control chars removed");
    char buf[] = "a\x01" "b\x02" "c";
    sanitize_string(buf, sizeof(buf));
    ASSERT(strcmp(buf, "abc") == 0, "");
    PASS();
}

static void test_carriage_return(void) {
    TEST("CR removed");
    char buf[] = "a\rb";
    sanitize_string(buf, sizeof(buf));
    ASSERT(strcmp(buf, "ab") == 0, "");
    PASS();
}

static void test_tab_kept(void) {
    TEST("tab preserved");
    char buf[] = "a\tb";
    sanitize_string(buf, sizeof(buf));
    ASSERT(strcmp(buf, "a\tb") == 0, "");
    PASS();
}

static void test_parse_int_valid(void) {
    TEST("parse valid int");
    ASSERT_EQ(parse_positive_int("42"), 42, "42");
    ASSERT_EQ(parse_positive_int("0"), 0, "0");
    PASS();
}

static void test_parse_int_negative(void) {
    TEST("parse negative rejected");
    ASSERT_EQ(parse_positive_int("-5"), 0, "");
    PASS();
}

static void test_parse_int_garbage(void) {
    TEST("parse garbage rejected");
    ASSERT_EQ(parse_positive_int("abc"), 0, "");
    ASSERT_EQ(parse_positive_int(""), 0, "");
    PASS();
}

int main(void) {
    printf("sanitize tests:\n");
    test_normal();
    test_control_chars();
    test_carriage_return();
    test_tab_kept();
    test_parse_int_valid();
    test_parse_int_negative();
    test_parse_int_garbage();

    printf("\n%d tests, %d failed\n", tests_run, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
