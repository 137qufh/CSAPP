include ../../common/flags.mk
CFLAGS += -I../../common/include -Iexercises

.PHONY: all test test-solution clean

DEMOS = demo_locality demo_stride demo_mountain demo_cache

all: $(DEMOS)

$(DEMOS):
	$(CC) $(CFLAGS) -o $@ $@.c

test_student: tests/test_locality.c exercises/ex_locality.c exercises/ex_locality.h
	$(CC) $(CFLAGS) -o test_student tests/test_locality.c exercises/ex_locality.c

test_solution_bin: tests/test_locality.c solutions/ex_locality.c exercises/ex_locality.h
	$(CC) $(CFLAGS) -o test_solution_bin tests/test_locality.c solutions/ex_locality.c

test: test_student
	./test_student

test-solution: test_solution_bin
	./test_solution_bin

clean:
	rm -f $(DEMOS) test_student test_solution_bin memory_mountain.csv memory_mountain.png
