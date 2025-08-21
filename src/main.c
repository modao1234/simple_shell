#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>
#include <string.h>

#include "parser.h"
#include "util.h"
#include "executor.h"
#include "builtin.h"
#include "jobs.h"

int main(void) {
    jobs_init();
    signal(SIGINT, SIG_IGN);

    char *line = NULL; size_t cap = 0; ssize_t n;

    while (1) {
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd))) printf("%s$ ", cwd); else printf("mini-shell$ ");
        fflush(stdout);

        n = getline(&line, &cap, stdin);
        if (n == -1) { putchar('\n'); break; }
        if (n && line[n-1]=='\n') line[n-1]='\0';
        trim(line); if (!*line) { jobs_reap(); continue; }

        pipeline_t p;
        if (parse_line(line, &p) != 0) { jobs_reap(); continue; }
        if (p.n_cmds == 0) { free_pipeline(&p); jobs_reap(); continue; }

        if (p.n_cmds == 1 && p.cmds[0].argv && is_builtin(&p.cmds[0])) {
            (void)do_builtin(&p.cmds[0]);
        } else {
            (void)exec_pipeline(&p);
        }
        free_pipeline(&p);

        jobs_reap();
    }
    free(line);
    return 0;
}