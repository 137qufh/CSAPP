include ../../common/flags.mk

CFLAGS += -I../../common/include

.PHONY: all test test-solution clean

all: defuse defuse_debug

# Student binary: -O1 and fully stripped. defuse_debug keeps symbols
# for the instructor and is not the assigned artifact.
defuse: solutions/defuse.c
	$(CC) $(CFLAGS) -O1 -s -o $@ solutions/defuse.c
	strip --strip-all $@

defuse_debug: solutions/defuse.c
	$(CC) -Wall -Wextra -std=gnu11 -g -O0 -fwrapv -D_GNU_SOURCE -o $@ solutions/defuse.c

test_student: tests/test_defuse.c
	$(CC) $(CFLAGS) -o $@ tests/test_defuse.c

test_solution_bin: tests/test_defuse.c
	$(CC) $(CFLAGS) -o $@ tests/test_defuse.c

# exercises/answers.txt is a placeholder, so this fails until it is solved.
test: defuse test_student
	./test_student exercises/answers.txt

test-solution: defuse test_solution_bin
	./test_solution_bin

clean:
	rm -f defuse defuse_debug test_student test_solution_bin
