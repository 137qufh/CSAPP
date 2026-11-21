include ../../common/flags.mk

CFLAGS += -I../../common/include -I.

.PHONY: all test test-solution clean

all: shell-student shell-solution

shell-student: shell_main.c parse.c exercises/eval.c
	$(CC) $(CFLAGS) -o $@ shell_main.c parse.c exercises/eval.c

shell-solution: shell_main.c parse.c solutions/eval.c
	$(CC) $(CFLAGS) -o $@ shell_main.c parse.c solutions/eval.c

test_student: tests/test_shell.c parse.c
	$(CC) $(CFLAGS) -o $@ tests/test_shell.c parse.c

test_solution_bin: tests/test_shell.c parse.c
	$(CC) $(CFLAGS) -o $@ tests/test_shell.c parse.c

test: shell-student test_student
	./test_student ./shell-student

test-solution: shell-solution test_solution_bin
	./test_solution_bin ./shell-solution

clean:
	rm -f shell-student shell-solution test_student test_solution_bin
