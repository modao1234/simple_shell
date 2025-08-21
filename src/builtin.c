#include "builtin.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int is_builtin(cmd_t *cmd) {
    if (!cmd || !cmd->argv || !cmd->argv[0]) return 0;
    return strcmp(cmd->argv[0], "cd") == 0 || strcmp(cmd->argv[0], "exit") == 0;
}

int do_builtin(cmd_t *cmd) {
    if (!is_builtin(cmd)) return 0;

    if (strcmp(cmd->argv[0], "cd") == 0) {
        const char *target = cmd->argv[1] ? cmd->argv[1] : getenv("HOME");
        if (!target) { fprintf(stderr, "cd: HOME not set\n"); return 1; }
        if (chdir(target) < 0) perror("cd");
        return 1;
    }
    if (strcmp(cmd->argv[0], "exit") == 0) {
        int code = 0;
        if (cmd->argv[1]) code = atoi(cmd->argv[1]);
        exit(code);
    }
    return 0;
}