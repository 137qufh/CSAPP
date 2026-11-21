include ../../common/flags.mk

CFLAGS += -I../../common/include -Iexercises

.PHONY: all test test-solution clean demos-run

BUILD := build

DEMOS = demo_fork demo_exec demo_wait demo_zombie demo_orphan \
	demo_signal demo_mask demo_suspend demo_setjmp

all: $(DEMOS)

$(DEMOS): %: %.c
	$(CC) $(CFLAGS) -o $@ $<

test_student: tests/test_ecf.c exercises/ex_forkcount.c exercises/ex_signals.c \
		exercises/ex_forkcount.h exercises/ex_signals.h
	$(CC) $(CFLAGS) -o $@ tests/test_ecf.c exercises/ex_forkcount.c exercises/ex_signals.c

test_solution_bin: tests/test_ecf.c solutions/ex_forkcount.c solutions/ex_signals.c \
		exercises/ex_forkcount.h exercises/ex_signals.h
	$(CC) $(CFLAGS) -o $@ tests/test_ecf.c solutions/ex_forkcount.c solutions/ex_signals.c

test: test_student
	./test_student

demos-run: all
	@set -eu; \
	mkdir -p $(BUILD); \
	for d in $(DEMOS); do \
		echo "RUN $$d"; \
		./$$d >$(BUILD)/$$d.out; \
	done; \
	grep -qx 'child' $(BUILD)/demo_fork.out; \
	grep -qx 'parent' $(BUILD)/demo_fork.out; \
	grep -qx 'echo-ok' $(BUILD)/demo_exec.out; \
	grep -qx 'code=3' $(BUILD)/demo_wait.out; \
	grep -qx 'code=5' $(BUILD)/demo_wait.out; \
	grep -Exq 'state=Z|state=not-Z|proc-unavailable' $(BUILD)/demo_zombie.out; \
	grep -Eq '^parent-pid=[0-9]+$$' $(BUILD)/demo_orphan.out; \
	grep -Eq '^child-sees=[0-9]+$$' $(BUILD)/demo_orphan.out; \
	grep -qx 'caught' $(BUILD)/demo_signal.out; \
	grep -qx 'pending=1' $(BUILD)/demo_mask.out; \
	grep -qx 'delivered' $(BUILD)/demo_mask.out; \
	grep -qx 'resumed' $(BUILD)/demo_suspend.out; \
	grep -qx 'setjmp' $(BUILD)/demo_setjmp.out; \
	grep -qx 'jumped=42' $(BUILD)/demo_setjmp.out; \
	echo "demos-run ok"

test-solution: test_solution_bin demos-run
	./test_solution_bin

clean:
	rm -rf $(BUILD)
	rm -f $(DEMOS) test_student test_solution_bin
