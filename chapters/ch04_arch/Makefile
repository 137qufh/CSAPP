include ../../common/flags.mk

CFLAGS += -I../../common/include -I. -Iexercises

.PHONY: all test test-solution clean

all: demo_seq demo_pipe demo_gates

demo_seq: demo_seq.c my86.c my86.h
	$(CC) $(CFLAGS) -o $@ demo_seq.c my86.c

demo_pipe: demo_pipe.c my86.c my86.h
	$(CC) $(CFLAGS) -o $@ demo_pipe.c my86.c

demo_gates: demo_gates.c
	$(CC) $(CFLAGS) -o $@ demo_gates.c

test_student: tests/test_my86.c tests/test_ex.c exercises/ex_hazard.c exercises/ex_adder.c my86.c
	$(CC) $(CFLAGS) -o $@ tests/test_ex.c exercises/ex_hazard.c exercises/ex_adder.c

test_solution_bin: tests/test_my86.c tests/test_ex.c solutions/ex_hazard.c solutions/ex_adder.c my86.c
	$(CC) $(CFLAGS) -o test_my86_sol tests/test_my86.c my86.c
	$(CC) $(CFLAGS) -o test_ex_sol tests/test_ex.c solutions/ex_hazard.c solutions/ex_adder.c

test: test_student
	./test_student

test-solution: test_solution_bin demo_seq demo_gates
	./test_my86_sol
	./test_ex_sol
	./demo_gates
	./demo_seq programs/sum.my >/tmp/my86-sum.out
	grep -q '^r1=55$$' /tmp/my86-sum.out

clean:
	rm -f demo_seq demo_pipe demo_gates test_student test_my86_sol test_ex_sol
