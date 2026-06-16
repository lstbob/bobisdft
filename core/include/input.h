#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include <stddef.h>

bool read_line(char *buf, size_t size);
bool read_line_nonempty(char *buf, size_t size);
bool confirm(const char *prompt);
void strip_newline(char *s);

#endif
