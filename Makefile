CC      := gcc
CFLAGS  := -std=gnu11 -Wall -Wextra -O2 -Iinclude
LDFLAGS :=

SRCS    := src/util.c src/parser.c src/executor.c src/builtin.c src/job.c src/main.c
OBJS    := $(SRCS:.c=.o)
TARGET  := mini_shell

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)