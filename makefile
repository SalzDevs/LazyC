CC = gcc
CFLAGS = -Wall -Wextra -g -pthread

SRCS = main.c lazy.c
EXEC = main

all: $(EXEC)

$(EXEC): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(EXEC)

clean:
	rm -f $(EXEC)
