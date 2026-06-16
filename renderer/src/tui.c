#include "tui.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <termios.h>
#include <sys/ioctl.h>

static struct termios orig_termios;
static bool raw_mode_active = false;

static void raw_mode_cleanup(int sig) {
    raw_mode_disable();
    (void)sig;
    _exit(128 + sig);
}

void raw_mode_enable(void) {
    if (raw_mode_active) return;
    struct termios raw;
    tcgetattr(STDIN_FILENO, &orig_termios);
    raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    raw_mode_active = true;

    signal(SIGINT, raw_mode_cleanup);
    signal(SIGTERM, raw_mode_cleanup);
    signal(SIGQUIT, raw_mode_cleanup);
}

void raw_mode_disable(void) {
    if (!raw_mode_active) return;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    raw_mode_active = false;
}

TermSize get_term_size(void) {
    TermSize ts = {24, 80};
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != -1) {
        ts.rows = ws.ws_row;
        ts.cols = ws.ws_col;
    }
    return ts;
}

int read_key(void) {
    char c;
    if (read(STDIN_FILENO, &c, 1) != 1) return KEY_NONE;

    if (c == '\033') {
        char seq[2];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return KEY_ESC;
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return KEY_ESC;

        if (seq[0] == '[') {
            switch (seq[1]) {
                case 'A': return KEY_UP;
                case 'B': return KEY_DOWN;
                case 'C': return KEY_RIGHT;
                case 'D': return KEY_LEFT;
            }
        }
        return KEY_ESC;
    }

    return (unsigned char)c;
}
