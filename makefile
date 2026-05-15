CC = gcc
CFLAGS = -Wall -Wextra -g -pthread

EXEC = main
TEST_EXEC = tests/test_lazy

all: $(EXEC)

$(EXEC): main.c lazy.c lazy.h
	$(CC) $(CFLAGS) main.c lazy.c -o $(EXEC)

$(TEST_EXEC): tests/test_lazy.c lazy.c lazy.h
	$(CC) $(CFLAGS) tests/test_lazy.c lazy.c -o $(TEST_EXEC)

test: $(TEST_EXEC)
	./$(TEST_EXEC)

clean:
	rm -f $(EXEC) $(TEST_EXEC)
