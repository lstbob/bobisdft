#include "editor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

/* Search $PATH for an executable named `name`. Returns true if found and
 * executable. This avoids hardcoding /usr/bin/nvim, which could grab a stale
 * distro nvim (e.g. 0.10.4) instead of /usr/local/bin/nvim (0.12.2). */
static bool find_in_path(const char *name) {
    const char *path = getenv("PATH");
    if (!path || !path[0]) return false;

    char buf[1024];
    const char *p = path;
    while (*p) {
        const char *colon = strchr(p, ':');
        size_t len = colon ? (size_t)(colon - p) : strlen(p);
        size_t name_len = strlen(name);
        if (len > 0 && len + 1 + name_len + 1 <= sizeof(buf)) {
            memcpy(buf, p, len);
            buf[len] = '/';
            memcpy(buf + len + 1, name, name_len + 1);
            if (access(buf, X_OK) == 0) return true;
        }
        if (!colon) break;
        p = colon + 1;
    }
    return false;
}

static const char *find_editor(void) {
    const char *env = getenv("EDITOR");
    if (env && env[0]) return env;

    /* Bare names => execlp() resolves via PATH, preferring /usr/local/bin/nvim
     * (or /opt/nvim) over a stale /usr/bin/nvim. */
    if (find_in_path("nvim")) return "nvim";
    if (find_in_path("vim"))  return "vim";
    return "vi";  /* ships with every Linux distro */
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
