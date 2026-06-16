CC       = gcc
CFLAGS   = -std=c11 -D_GNU_SOURCE -Wall -Wextra -Wpedantic -Werror -Wno-unused-result -O2 -D_FORTIFY_SOURCE=2
LDFLAGS  =
TARGET   = bobisdft

CORE_SRC  = $(wildcard core/src/*.c)
RENDER_SRC = $(wildcard renderer/src/*.c)
SRC       = main.c $(CORE_SRC) $(RENDER_SRC)
OBJ       = $(SRC:.c=.o)
INC       = -Icore/include -Irenderer/include

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET) core/tests/test_diary core/tests/test_date_utils core/tests/test_sanitize core/tests/test_storage

test: $(TARGET) core/tests/test_diary core/tests/test_date_utils core/tests/test_sanitize core/tests/test_storage
	@failed=0; \
	for t in core/tests/test_diary core/tests/test_date_utils core/tests/test_sanitize core/tests/test_storage; do \
		echo "=== $$t ==="; \
		./$$t || failed=1; \
		echo; \
	done; \
	exit $$failed

core/tests/test_diary: core/tests/test_diary.c core/src/diary.c
	$(CC) $(CFLAGS) $(INC) -o $@ $^

core/tests/test_date_utils: core/tests/test_date_utils.c core/src/date_utils.c core/src/diary.c
	$(CC) $(CFLAGS) $(INC) -o $@ $^

core/tests/test_sanitize: core/tests/test_sanitize.c core/src/sanitize.c
	$(CC) $(CFLAGS) $(INC) -o $@ $^

core/tests/test_storage: core/tests/test_storage.c core/src/storage.c core/src/date_utils.c core/src/diary.c core/src/input.c core/src/sanitize.c
	$(CC) $(CFLAGS) $(INC) -o $@ $^

.PHONY: all clean test
