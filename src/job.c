#define _POSIX_C_SOURCE 200809L
#include "jobs.h"
#include <signal.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>

static volatile sig_atomic_t got_sigchld = 0;

static void on_sigchld(int signo) {
    (void)signo;
    got_sigchld = 1;   /* 只置位，不在信号上下文里 waitpid，防止与前台竞争 */
}

void jobs_init(void) {
    struct sigaction sa = {0};
    sa.sa_handler = on_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);
}

void jobs_reap(void) {
    if (!got_sigchld) return;
    got_sigchld = 0;
    int status;
    while (1) {
        pid_t pid = waitpid(-1, &status, WNOHANG);
        if (pid <= 0) break;
        if (status >= 0) {
            if (WIFEXITED(status))
                fprintf(stderr, "[done] %d exit %d\n", (int)pid, WEXITSTATUS(status));
            else if (WIFSIGNALED(status))
                fprintf(stderr, "[done] %d sig %d\n", (int)pid, WTERMSIG(status));
        }
    }
}