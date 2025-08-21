#ifndef UTIL_H
#define UTIL_H
#include <stddef.h>

void trim(char *s);

void *xmalloc(size_t n);
void *xrealloc(void *p, size_t n);

char *strdup_safe(const char *s);

int safe_open_rdonly(const char *path);
int safe_open_trunc(const char *path);
int safe_open_append(const char *path);

char **vec_dup(char **v);
void vec_free(char **v);

#endif