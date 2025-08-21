#define _POSIX_C_SOURCE 200809L
#include "executor.h"
#include "util.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static void child_reset_signals(void) {
    struct sigaction sa = {0};
    sa.sa_handler = SIG_DFL; sigemptyset(&sa.sa_mask);
    sigaction(SIGINT,  &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGTSTP, &sa, NULL);
    sigaction(SIGCHLD, &sa, NULL);
    sigaction(SIGPIPE, &sa, NULL);
    sigset_t empty; sigemptyset(&empty);
    sigprocmask(SIG_SETMASK, &empty, NULL);
}

static void apply_redirs_or_die (const cmd_t *cmd) {
    int fd;
    if (cmd->infile) {
        fd = safe_open_rdonly(cmd->infile);
        if (fd < 0) _exit(1);
        if (dup2(fd, STDIN_FILENO) < 0) { perror("dup2 infile"); close(fd); _exit(1); }
        close(fd);
    }
    if (cmd->appendfile) {
        fd = safe_open_append(cmd->appendfile);
        if (fd < 0) _exit(1);
        if (dup2(fd, STDOUT_FILENO) < 0) { perror("dup2 append"); close(fd); _exit(1); }
        close(fd);
    } else if (cmd->outfile) {
        fd = safe_open_trunc(cmd->outfile);
        if (fd < 0) _exit(1);
        if (dup2(fd, STDOUT_FILENO) < 0) { perror("dup2 outfile"); close(fd); _exit(1); }
        close(fd);
    }
}

int exec_single(cmd_t *cmd, int foreground) {
    if (!cmd || !cmd->argv || !cmd->argv[0]) return 0;

    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return -1; }

    if (pid == 0) {
        child_reset_signals();
        apply_redirs_or_die(cmd);
        execvp(cmd->argv[0], cmd->argv);
        fprintf(stderr, "exec failed: %s: %s\n", cmd->argv[0], strerror(errno));
        _exit(127);
    }

    if (foreground) {
        int status = 0;
        if (waitpid(pid, &status, 0) < 0) { perror("waitpid"); return -1;}
        if (WIFEXITED(status)) return WEXITSTATUS(status);
        if (WIFSIGNALED(status)) return 128 + WTERMSIG(status);
        return 0;
    } else {
        fprintf(stderr, "[bg] %d\n", (int)pid);
        return 0;
    }
}

int exec_pipeline(pipeline_t *p) {
    if (!p || p->n_cmds <= 0) return 0;
    if (p->n_cmds == 1) return exec_single(&p->cmds[0], !p->background);

    int n = p->n_cmds;
    pid_t pids[n];
    memset(pids, 0, sizeof(pids));

    int prev_read = -1;
    pid_t pgid = 0;

    for (int i = 0; i < n; ++i) {
        int pipefd[2] = {-1, -1};
        if (i < n - 1) {
            if (pipe(pipefd) < 0) {
                perror("pipe");
                if (prev_read != -1) close(prev_read);
                goto fail_kill;
            }
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            if (pipefd[0] != -1) { close(pipefd[0]); close(pipefd[1]); }
            if (prev_read != -1) close(prev_read);
            goto fail_kill;
        }

        if (pid == 0) {
            child_reset_signals();

            if (!pgid) pgid = getpid();
            if (setpgid(0, pgid) < 0) { }

            if (prev_read != -1) {
                if (dup2(prev_read, STDIN_FILENO) < 0) { perror("dup2 prev_read"); _exit(1); }
            }
            if (i < n - 1) {
                if (dup2(pipefd[1], STDOUT_FILENO) < 0) { perror("dup2 pipe write"); _exit(1); }
            }

            if (prev_read != -1) close(prev_read);
            if (i < n - 1) { close(pipefd[0]); close(pipefd[1]); }

            apply_redirs_or_die(&p->cmds[i]);

            char **argv = p->cmds[i].argv;
            if (!argv || !argv[0]) _exit(0);
            execvp(argv[0], argv);
            fprintf(stderr, "exec failed: %s: %s\n", argv[0], strerror(errno));
            _exit(127);
        }

        pids[i] = pid;
        if (!pgid) pgid = pid;
        if (setpgid(pid, pgid) < 0) { }

        if (prev_read != -1) { close(prev_read); prev_read = -1; }
        if (i < n - 1) {
            close(pipefd[1]);
            prev_read = pipefd[0];
        }
    }

    if (!p->background) {
        int status, rc = 0;
        pid_t last = pids[n - 1];
        int remain = n;
        while (remain > 0) {
            pid_t w = waitpid(-1, &status, 0);
            if (w < 0) { if (errno == EINTR) continue; perror("waitpid"); break; }
            --remain;
            if (w == last) {
                if (WIFEXITED(status)) rc = WEXITSTATUS(status);
                else if (WIFSIGNALED(status)) rc = 128 + WTERMSIG(status);
            }
        }
        return rc;
    } else {
        fprintf(stderr, "[bg]");
        for (int i = 0; i < n; ++i) fprintf(stderr, " %d", (int)pids[i]);
        fprintf(stderr, "\n");
        return 0;
    }

fail_kill:
    if (pgid > 0) killpg(pgid, SIGTERM);
    for (int i = 0; i < n; ++i) {
        if (pids[i] > 0) waitpid(pids[i], NULL, 0);
    }
    return -1;
}