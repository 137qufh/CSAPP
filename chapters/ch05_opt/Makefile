include ../../common/flags.mk
CFLAGS += -I../../common/include -Iexercises

.PHONY: all test test-solution clean

DEMOS = demo_cpe demo_motion demo_accum demo_unroll

all: $(DEMOS)

$(DEMOS):
	$(CC) $(CFLAGS) -o $@ $@.c

test_student: tests/test_opt.c exercises/ex_opt.c exercises/ex_opt.h
	$(CC) $(CFLAGS) -o test_student tests/test_opt.c exercises/ex_opt.c

test_solution_bin: tests/test_opt.c solutions/ex_opt.c exercises/ex_opt.h
	$(CC) $(CFLAGS) -o test_solution_bin tests/test_opt.c solutions/ex_opt.c

test: test_student
	./test_student

test-solution: test_solution_bin
	./test_solution_bin

clean:
	rm -f $(DEMOS) test_student test_solution_bin
