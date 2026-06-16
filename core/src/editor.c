#include "editor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

static const char *find_editor(void) {
    const char *env = getenv("EDITOR");
    if (env && env[0]) return env;

    struct stat st;
    if (stat("/usr/bin/nvim", &st) == 0) return "/usr/bin/nvim";
    if (stat("/usr/bin/vim", &st) == 0) return "/usr/bin/vim";
    if (stat("/usr/bin/vi", &st) == 0)  return "/usr/bin/vi";

    return "vi";
}

bool open_editor(const char *initial_content, size_t initial_len,
                 char **out_content, size_t *out_len) {
    char template[] = "/tmp/bobisdft_XXXXXX";
    int fd = mkstemp(template);
    if (fd == -1) return false;

    if (initial_content && initial_len > 0) {
        FILE *f = fdopen(fd, "w");
        if (!f) { close(fd); unlink(template); return false; }
        fwrite(initial_content, 1, initial_len, f);
        fclose(f);
    } else {
        close(fd);
    }

    const char *editor = find_editor();
    pid_t pid = fork();

    if (pid == -1) {
        unlink(template);
        return false;
    }

    if (pid == 0) {
        execlp(editor, editor, template, (char *)NULL);
        _exit(1);
    }

    int status;
    waitpid(pid, &status, 0);

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        unlink(template);
        return false;
    }

    FILE *f = fopen(template, "rb");
    if (!f) { unlink(template); return false; }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (fsize <= 0) {
        fclose(f);
        unlink(template);
        *out_content = strdup("");
        *out_len = 0;
        return true;
    }

    *out_content = (char *)malloc((size_t)fsize + 1);
    if (!*out_content) { fclose(f); unlink(template); return false; }

    size_t nread = fread(*out_content, 1, (size_t)fsize, f);
    fclose(f);
    unlink(template);

    (*out_content)[nread] = '\0';
    *out_len = nread;
    return true;
}
