#ifndef EDITOR_H
#define EDITOR_H

#include <stdbool.h>
#include <stddef.h>

bool open_editor(const char *initial_content, size_t initial_len,
                 char **out_content, size_t *out_len);

#endif
