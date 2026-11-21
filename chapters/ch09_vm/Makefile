include ../../common/flags.mk

CFLAGS += -I../../common/include -Iexercises

.PHONY: all test test-solution clean

DEMOS = demo_maps demo_mmap demo_tlb demo_gc demo_explicit

all: $(DEMOS)

$(DEMOS):
	$(CC) $(CFLAGS) -o $@ $@.c

test_student: tests/test_addr.c exercises/ex_addr.c
	$(CC) $(CFLAGS) -o $@ $^

test_solution_bin: tests/test_addr.c solutions/ex_addr.c
	$(CC) $(CFLAGS) -o $@ $^

test: test_student
	./test_student

test-solution: test_solution_bin $(DEMOS)
	./test_solution_bin
	./demo_maps >/dev/null
	./demo_mmap
	./demo_tlb
	./demo_gc
	./demo_explicit

clean:
	rm -f $(DEMOS) test_student test_solution_bin
