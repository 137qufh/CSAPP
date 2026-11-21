include ../../common/flags.mk

CFLAGS += -I../../common/include -Iexercises

.PHONY: all test test-solution clean asm

C_DEMOS = demo_mov demo_arith demo_cc demo_loop demo_switch demo_call \
	demo_array demo_safe_stack

DEMOS = $(C_DEMOS) demo_asm

all: $(DEMOS)

$(C_DEMOS): %: %.c
	$(CC) $(CFLAGS) -o $@ $<

demo_asm: demo_asm.c asm/add_loop.s
	$(CC) $(CFLAGS) -o $@ demo_asm.c asm/add_loop.s

# Annotated assembly for reading. -Og keeps the structure of the C,
# frame pointers stay, and -fverbose-asm names the operands.
asm: demo_asm
	mkdir -p build
	@set -e; for src in $(DEMOS); do \
		echo "CC -S $$src.c -> build/$$src.s"; \
		$(CC) -Wall -Wextra -std=gnu11 -Og -S \
			-fno-asynchronous-unwind-tables -fno-omit-frame-pointer \
			-fverbose-asm -fwrapv -D_GNU_SOURCE \
			-I../../common/include -Iexercises \
			-o build/$$src.s $$src.c; \
	done

test_student: tests/test_layout.c exercises/ex_layout.c exercises/ex_layout.h
	$(CC) $(CFLAGS) -o $@ tests/test_layout.c exercises/ex_layout.c

test_solution_bin: tests/test_layout.c solutions/ex_layout.c exercises/ex_layout.h
	$(CC) $(CFLAGS) -o $@ tests/test_layout.c solutions/ex_layout.c

test: test_student
	./test_student

test-solution: test_solution_bin
	./test_solution_bin

clean:
	rm -f $(DEMOS) test_student test_solution_bin
	rm -rf build
