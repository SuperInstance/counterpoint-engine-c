CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99 -O2 -Iinclude

.PHONY: all test clean

all: build/counterpoint_engine.a

build:
	mkdir -p build

build/counterpoint_engine.o: src/counterpoint_engine.c include/counterpoint_engine.h | build
	$(CC) $(CFLAGS) -c $< -o $@

build/counterpoint_engine.a: build/counterpoint_engine.o
	ar rcs $@ $<

build/test_counterpoint: build/counterpoint_engine.o tests/test_counterpoint.c
	$(CC) $(CFLAGS) build/counterpoint_engine.o tests/test_counterpoint.c -lm -o $@

test: build/test_counterpoint
	./build/test_counterpoint

clean:
	rm -rf build
