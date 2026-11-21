include ../../common/flags.mk

CFLAGS += -I../../common/include -I.

.PHONY: all test test-solution clean

all: test_student test_solution_bin

test_student: tests/test_bits.c exercises/bits.c bits.h
	$(CC) $(CFLAGS) -o $@ tests/test_bits.c exercises/bits.c -lm

test_solution_bin: tests/test_bits.c solutions/bits.c bits.h
	$(CC) $(CFLAGS) -o $@ tests/test_bits.c solutions/bits.c -lm

test: test_student
	./test_student
	python3 opcheck.py exercises/bits.c

test-solution: test_solution_bin
	./test_solution_bin
	python3 opcheck.py solutions/bits.c

clean:
	rm -f test_student test_solution_bin
