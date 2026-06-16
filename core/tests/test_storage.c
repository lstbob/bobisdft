#include "storage.h"
#include "diary.h"
#include "date_utils.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>

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

/*
 * Write a raw .bobisd file with a correct header and recomputed checksum but
 * caller-supplied date/title/content bytes, so we can forge a structurally
 * valid yet hostile record. content_len is written verbatim (not derived from
 * content_nbytes) so we can simulate a forged length. MAGIC/VERSION mirror
 * storage.c intentionally.
 */
static void write_raw_entry(const char *path, Date date, const char *title,
                            uint32_t content_len, const void *content,
                            size_t content_nbytes) {
    uint8_t data[12 + MAX_TITLE_LEN + 4 + 4096];
    size_t pos = 0;
    memcpy(data + pos, &date.year, sizeof(int));  pos += sizeof(int);
    memcpy(data + pos, &date.month, sizeof(int)); pos += sizeof(int);
    memcpy(data + pos, &date.day, sizeof(int));   pos += sizeof(int);

    char title_buf[MAX_TITLE_LEN];
    memset(title_buf, 0, sizeof(title_buf));
    snprintf(title_buf, sizeof(title_buf), "%s", title ? title : "");
    memcpy(data + pos, title_buf, MAX_TITLE_LEN); pos += MAX_TITLE_LEN;

    memcpy(data + pos, &content_len, sizeof(content_len)); pos += sizeof(content_len);
    if (content && content_nbytes > 0) {
        memcpy(data + pos, content, content_nbytes);
        pos += content_nbytes;
    }

    uint8_t sum = 0;
    for (size_t k = 0; k < pos; k++) sum ^= data[k];

    FILE *f = fopen(path, "wb");
    if (!f) return;
    uint32_t magic = 0x44494152;
    uint8_t version = 1;
    fwrite(&magic, sizeof(magic), 1, f);
    fwrite(&version, sizeof(version), 1, f);
    fputc((int)sum, f);
    fwrite(data, 1, pos, f);
    fclose(f);
}

/* A content_len larger than the bytes actually present must be rejected, not
 * trusted into an oversized malloc + short read that exposes heap memory. */
static void test_forged_content_len(void) {
    TEST("rejects forged content_len");
    Date d = {2026, 8, 15};
    const char actual[] = "hi";   /* only 2 bytes present */
    const char *path = "data/2026-08-15.bobisd";
    write_raw_entry(path, d, "Title", 0x10000u, actual, sizeof(actual) - 1);

    DiaryEntry loaded;
    entry_clear(&loaded);
    ASSERT(!load_entry(d, &loaded), "forged content_len must be rejected");
    entry_free(&loaded);
    unlink(path);
    PASS();
}

/* A content_len smaller than the trailing bytes is also inconsistent. */
static void test_short_content_len(void) {
    TEST("rejects undersized content_len");
    Date d = {2026, 8, 16};
    const char actual[] = "abcdefgh";
    const char *path = "data/2026-08-16.bobisd";
    write_raw_entry(path, d, "Title", 2u, actual, sizeof(actual) - 1);

    DiaryEntry loaded;
    entry_clear(&loaded);
    ASSERT(!load_entry(d, &loaded), "inconsistent content_len must be rejected");
    entry_free(&loaded);
    unlink(path);
    PASS();
}

/* An honest record forged by hand must still load (sanity for the helper). */
static void test_raw_roundtrip(void) {
    TEST("hand-forged honest record loads");
    Date d = {2026, 8, 17};
    const char body[] = "valid body";
    const char *path = "data/2026-08-17.bobisd";
    write_raw_entry(path, d, "Hello", (uint32_t)(sizeof(body) - 1), body, sizeof(body) - 1);

    DiaryEntry loaded;
    entry_clear(&loaded);
    ASSERT(load_entry(d, &loaded), "consistent record should load");
    ASSERT_STR_EQ(loaded.title, "Hello", "title");
    ASSERT_STR_EQ(loaded.content, "valid body", "content");
    entry_free(&loaded);
    unlink(path);
    PASS();
}

/* An out-of-range month would index MONTH_NAMES[-1] in the renderer. */
static void test_bogus_date_rejected(void) {
    TEST("rejects out-of-range date");
    Date stored = {2026, 0, 15};   /* month 0 */
    const char *path = "data/2026-08-18.bobisd";
    write_raw_entry(path, stored, "T", 0u, NULL, 0);

    Date lookup = {2026, 8, 18};
    DiaryEntry loaded;
    entry_clear(&loaded);
    ASSERT(!load_entry(lookup, &loaded), "bogus stored date must be rejected");
    entry_free(&loaded);
    unlink(path);
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
    test_forged_content_len();
    test_short_content_len();
    test_raw_roundtrip();
    test_bogus_date_rejected();
    test_cleanup();

    printf("\n%d tests, %d failed\n", tests_run, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
