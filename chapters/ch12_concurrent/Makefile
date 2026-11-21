include ../../common/flags.mk

CFLAGS += -I../../common/include -Iexercises
CFLAGS += -pthread

.PHONY: all test test-solution clean

DEMOS = demo_proc_server demo_select demo_epoll demo_threads demo_race \
	demo_pc demo_rw demo_prethread demo_deadlock

all: $(DEMOS)

$(DEMOS): %: %.c
	$(CC) $(CFLAGS) -o $@ $<

test_student: tests/test_counter.c exercises/ex_counter.c
	$(CC) $(CFLAGS) -o $@ $^

test_solution_bin: tests/test_counter.c solutions/ex_counter.c
	$(CC) $(CFLAGS) -o $@ $^

test: test_student
	./test_student

test-solution: test_solution_bin $(DEMOS)
	./test_solution_bin
	./demo_proc_server
	./demo_select
	./demo_epoll
	./demo_threads
	./demo_race
	./demo_pc
	./demo_rw
	./demo_prethread
	./demo_deadlock

clean:
	rm -f $(DEMOS) test_student test_solution_bin
