#include "input.h"
#include "sanitize.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

void strip_newline(char *s) {
    size_t len = strlen(s);
    if (len > 0 && s[len - 1] == '\n') {
        s[len - 1] = '\0';
    }
}

bool read_line(char *buf, size_t size) {
    if (!fgets(buf, (int)size, stdin)) return false;
    strip_newline(buf);
    sanitize_string(buf, size);
    return true;
}

bool read_line_nonempty(char *buf, size_t size) {
    while (read_line(buf, size)) {
        if (buf[0] != '\0') return true;
    }
    return false;
}

bool confirm(const char *prompt) {
    char buf[16];
    while (1) {
        printf("%s [y/n]: ", prompt);
        fflush(stdout);
        if (!read_line(buf, sizeof(buf))) return false;
        char c = (char)tolower((unsigned char)buf[0]);
        if (c == 'y') return true;
        if (c == 'n') return false;
        printf("Please answer 'y' or 'n'.\n");
    }
}
