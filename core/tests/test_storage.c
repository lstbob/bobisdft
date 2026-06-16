#include "storage.h"
#include "diary.h"
#include "date_utils.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

static int tests_run = 0;
static int tests_failed = 0;

#define TEST(name) do { printf("  %s ... ", name); tests_run++; } while(0)
#define PASS() printf("ok\n")
#define FAIL(msg) do { printf("FAIL (%s)\n", msg); tests_failed++; } while(0)
#define ASSERT(cond, msg) do { if (!(cond)) { FAIL(msg); return; } } while(0)
#define ASSERT_EQ(a, b, msg) do { if ((a) != (b)) { FAIL(msg); return; } } while(0)
#define ASSERT_STR_EQ(a, b, msg) do { if (strcmp(a, b) != 0) { FAIL(msg); return; } } while(0)

static void test_init(void) {
    TEST("storage_init");
    rmdir("test_data");
    ASSERT(storage_init(), "init should succeed");
    PASS();
}

static void test_save_and_load(void) {
    TEST("save and load roundtrip");
    DiaryEntry e;
    entry_clear(&e);
    e.date = (Date){2026, 7, 4};
    snprintf(e.title, MAX_TITLE_LEN, "Independence Day");
    e.content = strdup("Celebrated with fireworks and barbecue.");
    e.content_len = strlen(e.content);

    ASSERT(save_entry(&e), "save");
    entry_free(&e);

    DiaryEntry loaded;
    entry_clear(&loaded);
    ASSERT(load_entry(e.date, &loaded), "load");

    ASSERT_EQ(loaded.date.year, 2026, "year");
    ASSERT_EQ(loaded.date.month, 7, "month");
    ASSERT_EQ(loaded.date.day, 4, "day");
    ASSERT_STR_EQ(loaded.title, "Independence Day", "title");
    ASSERT_STR_EQ(loaded.content, "Celebrated with fireworks and barbecue.", "content");

    entry_free(&loaded);
    PASS();
}

static void test_entry_exists(void) {
    TEST("entry_exists");
    Date d = {2026, 7, 4};
    ASSERT(entry_exists(d), "exists");
    Date missing = {2026, 1, 1};
    ASSERT(!entry_exists(missing), "not exists");
    PASS();
}

static void test_checksum_corruption(void) {
    TEST("checksum rejects corruption");
    char path[256];
    snprintf(path, sizeof(path), "data/%04d-%02d-%02d.bobisd", 2026, 7, 4);
    FILE *f = fopen(path, "r+b");
    ASSERT(f, "open");
    fseek(f, 100, SEEK_SET);
    fputc(0xFF, f);
    fclose(f);

    DiaryEntry loaded;
    entry_clear(&loaded);
    ASSERT(!load_entry((Date){2026, 7, 4}, &loaded), "rejected");
    PASS();
}

static void test_cleanup(void) {
    TEST("cleanup");
    unlink("data/2026-07-04.bobisd");
    rmdir("test_data");
    PASS();
}

int main(void) {
    printf("storage tests:\n");
    test_init();
    test_save_and_load();
    test_entry_exists();
    test_checksum_corruption();
    test_cleanup();

    printf("\n%d tests, %d failed\n", tests_run, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
