CC = gcc
CFLAGS = -Wall -Wextra -g -pthread
SANITIZE_FLAGS = $(CFLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer

EXEC = main
TEST_EXEC = tests/test_lazy
SANITIZE_EXEC = tests/test_lazy_sanitize

all: $(EXEC)

$(EXEC): main.c lazy.c lazy.h
	$(CC) $(CFLAGS) main.c lazy.c -o $(EXEC)

$(TEST_EXEC): tests/test_lazy.c lazy.c lazy.h
	$(CC) $(CFLAGS) tests/test_lazy.c lazy.c -o $(TEST_EXEC)

$(SANITIZE_EXEC): tests/test_lazy.c lazy.c lazy.h
	$(CC) $(SANITIZE_FLAGS) tests/test_lazy.c lazy.c -o $(SANITIZE_EXEC)

test: $(TEST_EXEC)
	./$(TEST_EXEC)

sanitize: $(SANITIZE_EXEC)
	./$(SANITIZE_EXEC)

clean:
	rm -f $(EXEC) $(TEST_EXEC) $(SANITIZE_EXEC)
