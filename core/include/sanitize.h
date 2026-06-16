#ifndef SANITIZE_H
#define SANITIZE_H

#include <stddef.h>

void sanitize_string(char *buf, size_t size);
int parse_positive_int(const char *s);

#endif
