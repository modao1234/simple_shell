#ifndef BUILTIN_H
#define BUILTIN_H
#include "parser.h"
int is_builtin(cmd_t *cmd);
int do_builtin(cmd_t *cmd);
#endif