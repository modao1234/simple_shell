#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>

typedef struct {
    char **argv;
    char *infile;
    char *outfile;
    char *appendfile;
} cmd_t;

typedef struct {
    cmd_t *cmds;
    int n_cmds;
    int background;
} pipeline_t;

int parse_line(const char *line, pipeline_t *out);
void free_pipeline(pipeline_t *p);

#endif
