#include "storage.h"
#include "date_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAGIC 0x44494152
#define VERSION 1

static uint8_t compute_checksum(const uint8_t *data, size_t len) {
    uint8_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum ^= data[i];
    }
    return sum;
}

static const char *file_path(Date date, char *buf, size_t buf_size) {
    snprintf(buf, buf_size, DATA_DIR "/%04d-%02d-%02d.bobisd",
             date.year, date.month, date.day);
    return buf;
}

bool storage_init(void) {
    struct stat st = {0};
    if (stat(DATA_DIR, &st) == -1) {
        if (mkdir(DATA_DIR, 0700) == -1) {
            return false;
        }
    }
    return true;
}

bool save_entry(const DiaryEntry *entry) {
    char path[256];
    file_path(entry->date, path, sizeof(path));

    FILE *f = fopen(path, "w+b");
    if (!f) return false;

    uint32_t magic = MAGIC;
    uint8_t version = VERSION;
    fwrite(&magic, sizeof(magic), 1, f);
    fwrite(&version, sizeof(version), 1, f);
    fputc(0, f);

    fwrite(&entry->date.year, sizeof(entry->date.year), 1, f);
    fwrite(&entry->date.month, sizeof(entry->date.month), 1, f);
    fwrite(&entry->date.day, sizeof(entry->date.day), 1, f);

    char title_buf[MAX_TITLE_LEN];
    memset(title_buf, 0, sizeof(title_buf));
    snprintf(title_buf, sizeof(title_buf), "%s", entry->title);
    fwrite(title_buf, 1, MAX_TITLE_LEN, f);

    uint32_t content_len = (uint32_t)entry->content_len;
    fwrite(&content_len, sizeof(content_len), 1, f);
    if (content_len > 0 && entry->content) {
        fwrite(entry->content, 1, content_len, f);
    }

    {
        long data_end = ftell(f);
        uint8_t *buf = (uint8_t *)malloc((size_t)data_end);
        if (buf) {
            fseek(f, 0, SEEK_SET);
            size_t nread = fread(buf, 1, (size_t)data_end, f);
            if (nread > 0) {
                uint8_t sum = compute_checksum(buf + 6, nread - 6);
                fseek(f, 5, SEEK_SET);
                fwrite(&sum, 1, 1, f);
            }
            free(buf);
        }
    }

    fclose(f);
    return true;
}

bool load_entry(Date date, DiaryEntry *out) {
    char path[256];
    file_path(date, path, sizeof(path));

    FILE *f = fopen(path, "rb");
    if (!f) return false;

    uint32_t magic;
    uint8_t version;
    if (fread(&magic, sizeof(magic), 1, f) != 1 || magic != MAGIC) {
        fclose(f);
        return false;
    }
    if (fread(&version, sizeof(version), 1, f) != 1 || version != VERSION) {
        fclose(f);
        return false;
    }

    uint8_t stored_checksum;
    if (fread(&stored_checksum, 1, 1, f) != 1) {
        fclose(f);
        return false;
    }

    long data_start = ftell(f);
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, data_start, SEEK_SET);

    size_t data_len = (size_t)(file_size - data_start);
    uint8_t *data = (uint8_t *)malloc(data_len + 1);
    if (!data) {
        fclose(f);
        return false;
    }
    size_t nread = fread(data, 1, data_len, f);
    if (nread != data_len || compute_checksum(data, data_len) != stored_checksum) {
        free(data);
        fclose(f);
        return false;
    }

    fseek(f, (long)data_start, SEEK_SET);
    entry_clear(out);

    fread(&out->date.year, sizeof(out->date.year), 1, f);
    fread(&out->date.month, sizeof(out->date.month), 1, f);
    fread(&out->date.day, sizeof(out->date.day), 1, f);

    char title_buf[MAX_TITLE_LEN];
    fread(title_buf, 1, MAX_TITLE_LEN, f);
    title_buf[MAX_TITLE_LEN - 1] = '\0';
    snprintf(out->title, MAX_TITLE_LEN, "%s", title_buf);

    uint32_t content_len;
    fread(&content_len, sizeof(content_len), 1, f);

    if (content_len > 0) {
        out->content = (char *)malloc((size_t)content_len + 1);
        if (out->content) {
            fread(out->content, 1, content_len, f);
            out->content[content_len] = '\0';
            out->content_len = content_len;
        }
    }

    free(data);
    fclose(f);
    return true;
}

bool entry_exists(Date date) {
    char path[256];
    file_path(date, path, sizeof(path));
    return access(path, F_OK) == 0;
}

int list_entries_in_range(Date start, Date end, Date *out, size_t max_count) {
    int count = 0;
    Date current = start;

    while (date_cmp(current, end) <= 0 && (size_t)count < max_count) {
        if (entry_exists(current)) {
            out[count] = current;
            count++;
        }
        current = add_days(current, 1);
    }

    return count;
}
