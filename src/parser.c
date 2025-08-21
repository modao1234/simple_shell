#include "parser.h"
#include "util.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_PARTS 64
#define MAX_TOKS 256

static int parse_single_cmd(char *s, cmd_t *cmd) {
    memset(cmd, 0, sizeof(cmd_t));
    trim(s);
    if (*s == '\0') return -1;

    char *saveptr = NULL;
    char *tok = strtok_r(s, " \t", &saveptr);
    char *toks[MAX_TOKS];
    int tcount = 0;

    while (tok && tcount < MAX_TOKS - 1) {
        toks[tcount] = tok;
        tok = strtok_r(NULL, " \t", &saveptr);
        tcount++;
    }
    toks[tcount] = NULL;

    if(tcount == 0) {
        cmd->argv = xmalloc(sizeof(char *));
        cmd->argv[0] = NULL;
        return 0;
    }

    char **argv = xmalloc((tcount + 1) * sizeof(char *));
    int argc = 0;

    for (int i = 0; i < tcount; i++) {
        if (strcmp(toks[i], "<") == 0){
            if (i+1 >= tcount) { fprintf(stderr, "parse error: expected file after '<'\n"); free(argv); return -1;}
            cmd->infile = strdup_safe(toks[++i]);
        } else if (strcmp(toks[i], ">") == 0){
            if (i+1 >= tcount) { fprintf(stderr, "parse error: expected file after '<'\n"); free(argv); return -1;}
            cmd->outfile = strdup_safe(toks[++i]);
        } else if (strcmp(toks[i], ">>") == 0){
            if (i+1 >= tcount) { fprintf(stderr, "parse error: expected file after '<'\n"); free(argv); return -1;}
            cmd->appendfile = strdup_safe(toks[++i]);
        } else {
            argv[argc++] = strdup_safe(toks[i]);
        }
    }
    argv[argc] = NULL;
    cmd->argv = argv;
    return 0;
}

int parse_line(const char *line, pipeline_t *out) {
    if (!line || !out) return -1;
    out->cmds = NULL;
    out->n_cmds = 0;
    out->background = 0;

    char *buf = strdup_safe(line);
    trim(buf);
    size_t L = strlen(buf);
    if (L == 0) { free(buf); return 0; }

    if (L > 0 && buf[L-1] == '&') {
        out->background = 1;
        buf[L-1] = '\0';
        trim(buf);
    }

    char *parts[MAX_PARTS];
    int nparts = 0;
    char *saveptr = NULL;
    char *p = strtok_r(buf, "|", &saveptr);
    while (p && nparts < MAX_PARTS) {
        parts[nparts++] = p;
        p =strtok_r(NULL, "|", &saveptr);
    }
    if (nparts == 0) { free(buf); return 0; }

    cmd_t *cmds = xmalloc(nparts * sizeof(cmd_t));
    int err = 0;
    for (int i = 0; i < nparts; ++i) {
        if (parse_single_cmd(parts[i], &cmds[i]) < 0) {
            err = 1;
            for (int j = 0; j < i; ++j) {
                if (cmds[j].argv) vec_free(cmds[j].argv);
                if (cmds[j].infile) free(cmds[j].infile);
                if (cmds[j].outfile) free(cmds[j].outfile);
                if (cmds[j].appendfile) free(cmds[j].appendfile);
            }
            free(cmds);
            break;
        }
    }

    if (err) { free(buf); return -1; }

    out->cmds = cmds;
    out->n_cmds = nparts;

    free(buf);
    return 0;
}

void free_pipeline(pipeline_t *p) {
    if (!p) return;
    for (int i = 0; i < p->n_cmds; ++i) {
        cmd_t *c =&p->cmds[i];
        if (c->argv) vec_free(c->argv);
        if (c->infile) free(c->infile);
        if (c->outfile) free(c->outfile);
        if (c->appendfile) free(c->appendfile);
    }
    free(p->cmds);
    p->cmds = NULL;
    p->n_cmds = 0;
    p->background = 0;
}



