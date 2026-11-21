include ../../common/flags.mk

CFLAGS += -I../../common/include -Iexercises

SC_LIB := ../../common/libscraft.a

.PHONY: all test test-solution clean

DEMOS = demo_rw demo_short demo_stat demo_dir demo_dup2 demo_compare

all: $(DEMOS)

demo_rw: demo_rw.c
	$(CC) $(CFLAGS) -o $@ $<

demo_short: demo_short.c $(SC_LIB)
	$(CC) $(CFLAGS) -o $@ demo_short.c $(SC_LIB)

demo_stat: demo_stat.c
	$(CC) $(CFLAGS) -o $@ $<

demo_dir: demo_dir.c
	$(CC) $(CFLAGS) -o $@ $<

demo_dup2: demo_dup2.c
	$(CC) $(CFLAGS) -o $@ $<

demo_compare: demo_compare.c
	$(CC) $(CFLAGS) -o $@ $<

$(SC_LIB):
	$(MAKE) -C ../../common all

test_student: tests/test_copy.c exercises/ex_copy.c
	$(CC) $(CFLAGS) -o $@ $^

test_solution_bin: tests/test_copy.c solutions/ex_copy.c
	$(CC) $(CFLAGS) -o $@ $^

test: test_student
	./test_student

test-solution: test_solution_bin $(DEMOS)
	./test_solution_bin
	./demo_rw
	./demo_short
	./demo_stat
	./demo_dir
	./demo_dup2
	./demo_compare

clean:
	rm -f $(DEMOS) test_student test_solution_bin
