#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "storage.h"
#include "commands.h"

#define VERSION "0.1.0"

static void print_usage(void) {
    printf("bobisdft -- personal diary v%s\n", VERSION);
    printf("Usage:\n");
    printf("  bobisdft ne          New entry (default: tomorrow)\n");
    printf("  bobisdft ne -w       New entries for the week\n");
    printf("  bobisdft se          Show entries grid\n");
    printf("  bobisdft se -d DATE  Show grid starting at DATE\n");
    printf("  bobisdft ee          Edit existing entry\n");
    printf("  bobisdft ee -d DATE  Edit entry for DATE\n");
    printf("  bobisdft -h, --help  Show this help\n");
    printf("  bobisdft -v, --version  Show version\n");
}

int main(int argc, char *argv[]) {
    if (!storage_init()) {
        fprintf(stderr, "error: could not initialize data storage\n");
        return 1;
    }

    if (argc < 2) {
        print_usage();
        return 1;
    }

    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_usage();
        return 0;
    }

    if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0) {
        printf("bobisdft v%s\n", VERSION);
        return 0;
    }

    if (strcmp(argv[1], "ne") == 0) {
        return cmd_ne(argc, argv);
    }

    if (strcmp(argv[1], "se") == 0) {
        return cmd_se(argc, argv);
    }

    if (strcmp(argv[1], "ee") == 0) {
        return cmd_ee(argc, argv);
    }

    print_usage();
    return 1;
}
