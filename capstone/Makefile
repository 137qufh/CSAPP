include ../common/flags.mk

CFLAGS += -I../common/include -I../chapters/ch04_arch -I../labs/cachecraft -I../labs/allocraft -pthread
LDFLAGS += -pthread

SRCS = main.c link.c memsys.c \
	../chapters/ch04_arch/my86.c \
	../labs/cachecraft/solutions/csim_core.c \
	../labs/allocraft/solutions/mm.c \
	../labs/allocraft/memlib.c

.PHONY: all run test test-solution clean

all: syscraft

syscraft: $(SRCS)
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(LDFLAGS)

run: syscraft
	./syscraft --demo

test: run
test-solution: run

clean:
	rm -f syscraft
