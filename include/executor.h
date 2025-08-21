#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "parser.h"
int exec_single(cmd_t *cmd, int foreground);
int exec_pipeline(pipeline_t *p);

#endif


