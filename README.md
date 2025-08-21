# simple\_shell

A minimal, modular Unix-like shell written in C.

## Features

- Prompt + line reading (`getline`).
- Basic parsing: whitespace tokenization, pipes (`|`), redirections (`<`, `>`, `>>`), trailing background marker (`&`).
- Builtins: `cd`, `exit`, plus small extras `pwd` and `echo [-n]`.
- Execute external programs with `fork()` + `execvp()` (foreground waits, background returns immediately).
- Redirections via `open()` + `dup2()`.
- Pipelines of arbitrary length using `pipe()`.
- Background jobs (`&`) with non-blocking reaping via `SIGCHLD`.
- Signals: the shell (parent) ignores `SIGINT` (Ctrl-C); children reset to defaults so they can be interrupted.



## Build

```bash
make          # build release
make run      # build and run
make clean
```

## Run

```bash
./mini_shell
```

## Quick usage

```text
$ pwd
$ echo hello
$ echo hello > out.txt
$ cat < out.txt
$ ls -l | grep '\\.c' | wc -l
$ sleep 3 &
$ cd /tmp
$ exit
```


