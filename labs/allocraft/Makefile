include ../../common/flags.mk

CFLAGS += -I../../common/include -I.

.PHONY: all test test-solution clean

all: driver-solution driver-student

driver-solution: tests/driver.c solutions/mm.c memlib.c
	$(CC) $(CFLAGS) -o $@ tests/driver.c solutions/mm.c memlib.c

driver-student: tests/driver.c exercises/mm.c memlib.c
	$(CC) $(CFLAGS) -o $@ tests/driver.c exercises/mm.c memlib.c

test: driver-student
	./driver-student

test-solution: driver-solution
	./driver-solution

clean:
	rm -f driver-solution driver-student
