include ../../common/flags.mk

CFLAGS += -I../../common/include -Iexercises

.PHONY: all test test-solution clean pipeline elf

all: demo_hello demo_amdahl

demo_hello: demo_hello.c
	$(CC) $(CFLAGS) -o $@ $<

demo_amdahl: demo_amdahl.c
	$(CC) $(CFLAGS) -o $@ $< -lm

pipeline: demo_hello.c
	bash scripts/pipeline.sh

elf: demo_hello
	bash scripts/elf_peek.sh

test_student: tests/test_amdahl.c exercises/ex_amdahl.c
	$(CC) $(CFLAGS) -o $@ $^ -lm

test_solution_bin: tests/test_amdahl.c solutions/ex_amdahl.c
	$(CC) $(CFLAGS) -o $@ $^ -lm

test: test_student
	./test_student

test-solution: test_solution_bin
	./test_solution_bin

clean:
	rm -f demo_hello demo_amdahl demo_hello.i demo_hello.s demo_hello.o demo_hello_pipe \
		test_student test_solution_bin
