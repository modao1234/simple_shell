#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

void trim(char *s) {
    if (!s) return;
    char *p = s;
    while (*p && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
    size_t len = strlen(s);
    while (len > 0 && (s[len-1] == ' ' || s[len-1] == '\t' || s[len-1] == '\n' || s[len-1] == '\r')) {
        s[len-1] = '\0';
        --len;
    }
}

void *xmalloc(size_t n)  {
    void *p = malloc(n);
    if (!p && n != 0) {
        perror("malloc");
        exit(1);
    }
    return p;
}

void *xrealloc(void *p, size_t n) {
    void *q = realloc(p, n);
    if (!q && n != 0) {
        perror("realloc");
        exit(1);
    }
    return q;
}

char *strdup_safe(const char *s) {
    if (!s) return NULL;
    char *d = strdup(s);
    if (!d) {
        perror("strdup");
        exit(1);
    }
    return d;
}

int safe_open_rdonly(const char *path) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "open('%s') failed: %s\n", path, strerror(errno));
        return -1;
    }
return fd;
}

int safe_open_trunc(const char *path) {
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        fprintf(stderr, "open('%s') failed: %s\n", path, strerror(errno));
        return -1;
    }
    return fd;
}

int safe_open_append(const char *path) {
    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) {
        fprintf(stderr, "open('%s') failed: %s\n", path, strerror(errno));
        return -1;
    }
    return fd;
}

static size_t vec_len(char **v) {
    if (!v) return 0;
    size_t n = 0;
    while (v[n]) ++n;
    return n;
}

char **vec_dup(char **v) {
    if (!v) return NULL;
    size_t n = vec_len(v);
    char **out = xmalloc((n + 1) * sizeof(char *));
    for (size_t i = 0; i < n; ++i) {
        out[i] = strdup_safe(v[i]);
    }
    out[n] = NULL;
    return out;
}

void vec_free(char **v) {
    if (!v) return;
    for (size_t i = 0; v[i]; ++i) free(v[i]);
    free (v);
}