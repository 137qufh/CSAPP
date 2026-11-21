include ../../common/flags.mk
CFLAGS += -I../../common/include -Iexercises
CFLAGS += -I.

.PHONY: all test test-solution clean

all: csim csim-ref

csim: tests/driver.c exercises/csim_core.c cache.h
	$(CC) $(CFLAGS) -o $@ tests/driver.c exercises/csim_core.c

csim-ref: tests/driver.c solutions/csim_core.c cache.h
	$(CC) $(CFLAGS) -o $@ tests/driver.c solutions/csim_core.c

test_student: tests/test_cache.c exercises/csim_core.c exercises/transpose.c cache.h transpose.h
	$(CC) $(CFLAGS) -o test_student tests/test_cache.c exercises/csim_core.c exercises/transpose.c

test_solution_bin: tests/test_cache.c solutions/csim_core.c solutions/transpose.c cache.h transpose.h
	$(CC) $(CFLAGS) -o test_solution_bin tests/test_cache.c solutions/csim_core.c solutions/transpose.c

test: test_student
	./test_student

test-solution: test_solution_bin
	./test_solution_bin

clean:
	rm -f csim csim-ref test_student test_solution_bin
