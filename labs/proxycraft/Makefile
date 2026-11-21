include ../../common/flags.mk

CFLAGS += -I../../common/include -pthread
LDFLAGS += -pthread

.PHONY: all test test-solution clean

all: proxy-solution proxy-student origin

origin: tests/origin.c
	$(CC) $(CFLAGS) -o $@ tests/origin.c

proxy-solution: solutions/proxy.c
	$(CC) $(CFLAGS) -o $@ solutions/proxy.c $(LDFLAGS)

proxy-student: exercises/proxy.c
	$(CC) $(CFLAGS) -o $@ exercises/proxy.c $(LDFLAGS)

test_solution_bin: tests/test_proxy.c
	$(CC) $(CFLAGS) -o $@ tests/test_proxy.c

test: proxy-student origin test_solution_bin
	./test_solution_bin ./origin ./proxy-student

test-solution: proxy-solution origin test_solution_bin
	./test_solution_bin ./origin ./proxy-solution

clean:
	rm -f proxy-solution proxy-student origin test_solution_bin
