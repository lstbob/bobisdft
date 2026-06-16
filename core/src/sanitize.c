#include "sanitize.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

void sanitize_string(char *buf, size_t size) {
    size_t write = 0;
    for (size_t read = 0; read < size && buf[read] != '\0'; read++) {
        unsigned char c = (unsigned char)buf[read];
        if (c == '\r') continue;
        if (c == '\n') continue;
        if (c < 32 && c != '\t') continue;
        buf[write++] = buf[read];
    }
    if (write < size) buf[write] = '\0';
}

int parse_positive_int(const char *s) {
    if (!s || s[0] == '\0') return 0;

    char *end = NULL;
    long val = strtol(s, &end, 10);
    if (end == s || *end != '\0') return 0;
    if (val < 0) return 0;
    if (val > 1000000) return 0;

    return (int)val;
}
