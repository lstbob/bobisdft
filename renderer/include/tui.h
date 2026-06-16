#ifndef TUI_H
#define TUI_H

#include <stdbool.h>

typedef struct {
    int rows;
    int cols;
} TermSize;

TermSize get_term_size(void);
void raw_mode_enable(void);
void raw_mode_disable(void);
int read_key(void);

typedef enum {
    KEY_NONE = 0,
    KEY_UP = -1,
    KEY_DOWN = -2,
    KEY_LEFT = -3,
    KEY_RIGHT = -4,
    KEY_ESC = -5,
    KEY_TAB = -6
} SpecialKey;

/* ANSI color codes */
#define ANSI_RESET   "\033[0m"
#define ANSI_BOLD    "\033[1m"
#define ANSI_DIM     "\033[2m"
#define ANSI_RED     "\033[31m"
#define ANSI_GREEN   "\033[32m"
#define ANSI_YELLOW  "\033[33m"
#define ANSI_BLUE    "\033[34m"
#define ANSI_MAGENTA "\033[35m"
#define ANSI_CYAN    "\033[36m"
#define ANSI_WHITE   "\033[37m"
#define ANSI_REVERSE "\033[7m"

#endif
