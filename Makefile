CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
AR = ar

SRC = src/counterpoint_engine.c
OBJ = $(SRC:.c=.o)
LIB = libcounterpoint_engine.a
HDR = include/counterpoint_engine.h

.PHONY: all lib test clean

all: lib

lib: $(LIB)

$(LIB): $(OBJ)
	$(AR) rcs $@ $^

src/%.o: src/%.c $(HDR)
	$(CC) $(CFLAGS) -Iinclude -c -o $@ $<

test: $(LIB) tests/test_counterpoint.c $(HDR)
	$(CC) $(CFLAGS) -Iinclude -o test_counterpoint tests/test_counterpoint.c $(LIB) -lm
	./test_counterpoint

clean:
	rm -f src/*.o $(LIB) test_counterpoint
