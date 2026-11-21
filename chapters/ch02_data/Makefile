include ../../common/flags.mk

CFLAGS += -I../../common/include -Iexercises

.PHONY: all test test-solution clean

DEMOS = demo_convert demo_bytes demo_int demo_shift demo_float

all: $(DEMOS)

$(DEMOS):
	$(CC) $(CFLAGS) -o $@ $@.c

test_student: tests/test_overflow.c exercises/ex_overflow.c tests/test_floatbits.c exercises/ex_floatbits.c
	$(CC) $(CFLAGS) -o test_ov_student tests/test_overflow.c exercises/ex_overflow.c
	$(CC) $(CFLAGS) -o test_fl_student tests/test_floatbits.c exercises/ex_floatbits.c

test_solution_bin: tests/test_overflow.c solutions/ex_overflow.c tests/test_floatbits.c solutions/ex_floatbits.c
	$(CC) $(CFLAGS) -o test_ov_solution tests/test_overflow.c solutions/ex_overflow.c
	$(CC) $(CFLAGS) -o test_fl_solution tests/test_floatbits.c solutions/ex_floatbits.c

test: test_student
	./test_ov_student
	./test_fl_student

test-solution: test_solution_bin
	./test_ov_solution
	./test_fl_solution

clean:
	rm -f $(DEMOS) test_ov_student test_fl_student test_ov_solution test_fl_solution
