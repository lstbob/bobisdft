#include "editor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

static int tests_run = 0;
static int tests_failed = 0;

#define TEST(name) do { printf("  %s ... ", name); tests_run++; } while(0)
#define PASS() printf("ok\n")
#define FAIL(msg) do { printf("FAIL (%s)\n", msg); tests_failed++; } while(0)
#define ASSERT(cond, msg) do { if (!(cond)) { FAIL(msg); return; } } while(0)
#define ASSERT_EQ(a, b, msg) do { if ((a) != (b)) { FAIL(msg); return; } } while(0)
#define ASSERT_STR_EQ(a, b, msg) do { if (strcmp(a, b) != 0) { FAIL(msg); return; } } while(0)

static void test_edit_with_true(void) {
    TEST("edit with true (no-op)");
    char *out = NULL;
    size_t len = 0;
    setenv("EDITOR", "/usr/bin/true", 1);
    int ok = open_editor("hello world", 11, &out, &len);
    ASSERT(ok, "should succeed");
    ASSERT_STR_EQ(out, "hello world", "content preserved");
    free(out);
    PASS();
}

static void test_edit_empty_content(void) {
    TEST("edit with empty initial content");
    char *out = NULL;
    size_t len = 0;
    setenv("EDITOR", "/usr/bin/true", 1);
    int ok = open_editor("", 0, &out, &len);
    ASSERT(ok, "should succeed");
    ASSERT_STR_EQ(out, "", "empty result");
    ASSERT_EQ(len, (size_t)0, "zero length");
    free(out);
    PASS();
}

static void test_edit_null_initial(void) {
    TEST("edit with NULL initial");
    char *out = NULL;
    size_t len = 0;
    setenv("EDITOR", "/usr/bin/true", 1);
    int ok = open_editor(NULL, 0, &out, &len);
    ASSERT(ok, "should succeed");
    ASSERT_STR_EQ(out, "", "empty result");
    free(out);
    PASS();
}

static void test_edit_modify_content(void) {
    TEST("edit that modifies content");
    char *out = NULL;
    size_t len = 0;

    char script[] = "/tmp/bobisdft_test_editor.sh";
    FILE *sf = fopen(script, "w");
    ASSERT(sf, "create script");
    fprintf(sf, "#!/bin/sh\necho 'modified content' > \"$1\"\n");
    fclose(sf);
    chmod(script, 0755);

    setenv("EDITOR", script, 1);
    int ok = open_editor("original", 8, &out, &len);
    ASSERT(ok, "should succeed");
    ASSERT_STR_EQ(out, "modified content\n", "content modified");
    free(out);
    unlink(script);

    /* also test with the appended approach */
    char script2[] = "/tmp/bobisdft_test_editor2.sh";
    sf = fopen(script2, "w");
    ASSERT(sf, "create script2");
    fprintf(sf, "#!/bin/sh\necho ' appended' >> \"$1\"\n");
    fclose(sf);
    chmod(script2, 0755);

    setenv("EDITOR", script2, 1);
    ok = open_editor("base", 4, &out, &len);
    ASSERT(ok, "should succeed");
    ASSERT_STR_EQ(out, "base appended\n", "content appended");
    free(out);
    unlink(script2);

    PASS();
}

static void test_edit_bad_editor(void) {
    TEST("edit with failing editor");
    char *out = NULL;
    size_t len = 0;
    setenv("EDITOR", "/usr/bin/false", 1);
    int ok = open_editor("content", 7, &out, &len);
    ASSERT(!ok, "should fail");
    PASS();
}

int main(void) {
    printf("editor tests:\n");
    test_edit_with_true();
    test_edit_empty_content();
    test_edit_null_initial();
    test_edit_modify_content();
    test_edit_bad_editor();

    printf("\n%d tests, %d failed\n", tests_run, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
