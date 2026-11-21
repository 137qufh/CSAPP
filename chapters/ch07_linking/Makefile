include ../../common/flags.mk

CFLAGS += -I../../common/include -Iexercises -Isrc

.PHONY: all test test-solution clean reloc demos-run

BUILD := build

all: demo_static_multi demo_weak demo_ar demo_so demo_dlopen \
	demo_interpose_compile demo_interpose_wrap demo_preload_target \
	libscgreet.a libscgreet.so libpreload.so

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/main.o: src/main.c src/ops.h | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ src/main.c

$(BUILD)/add.o: src/add.c src/ops.h | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ src/add.c

$(BUILD)/mul.o: src/mul.c src/ops.h | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ src/mul.c

demo_static_multi: $(BUILD)/main.o $(BUILD)/add.o $(BUILD)/mul.o
	$(CC) $(CFLAGS) -o $@ $(BUILD)/main.o $(BUILD)/add.o $(BUILD)/mul.o

$(BUILD)/strong_value.o: src/strong_value.c | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/weak_value.o: src/weak_value.c | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/weak_main.o: src/weak_main.c | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

demo_weak: $(BUILD)/weak_main.o $(BUILD)/strong_value.o $(BUILD)/weak_value.o
	$(CC) $(CFLAGS) -o $@ $(BUILD)/weak_main.o $(BUILD)/strong_value.o $(BUILD)/weak_value.o

$(BUILD)/greet.o: src/greet.c src/greet.h | $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ src/greet.c

libscgreet.a: $(BUILD)/greet.o
	ar rcs $@ $(BUILD)/greet.o

$(BUILD)/greet_pic.o: src/greet.c src/greet.h | $(BUILD)
	$(CC) $(CFLAGS) -fPIC -c -o $@ src/greet.c

libscgreet.so: $(BUILD)/greet_pic.o
	$(CC) $(CFLAGS) -shared -Wl,-soname,libscgreet.so -o $@ $(BUILD)/greet_pic.o

demo_ar: src/demo_ar.c src/greet.h libscgreet.a
	$(CC) $(CFLAGS) -o $@ src/demo_ar.c libscgreet.a

demo_so: src/demo_so.c src/greet.h libscgreet.so
	$(CC) $(CFLAGS) -o $@ src/demo_so.c -L. -lscgreet -Wl,-rpath,'$$ORIGIN'

demo_dlopen: src/demo_dlopen.c libscgreet.so
	$(CC) $(CFLAGS) -o $@ src/demo_dlopen.c -ldl

demo_interpose_compile: src/demo_interpose_compile.c src/count_malloc.h
	$(CC) $(CFLAGS) -o $@ src/demo_interpose_compile.c

demo_interpose_wrap: src/demo_interpose_wrap.c
	$(CC) $(CFLAGS) -fno-builtin -o $@ src/demo_interpose_wrap.c -Wl,--wrap,malloc

libpreload.so: src/preload.c
	$(CC) $(CFLAGS) -fPIC -shared -o $@ src/preload.c -ldl

demo_preload_target: src/demo_preload_target.c
	$(CC) $(CFLAGS) -fno-builtin -o $@ src/demo_preload_target.c

reloc:
	bash scripts/reloc_peek.sh

test_student: tests/test_symbols.c exercises/ex_symbols.c exercises/ex_symbols.h
	$(CC) $(CFLAGS) -o $@ tests/test_symbols.c exercises/ex_symbols.c

test_solution_bin: tests/test_symbols.c solutions/ex_symbols.c exercises/ex_symbols.h
	$(CC) $(CFLAGS) -o $@ tests/test_symbols.c solutions/ex_symbols.c

test: test_student
	./test_student

demos-run: all
	@set -eu; \
	echo "RUN demo_static_multi"; \
	test "$$(./demo_static_multi)" = "sum=7 product=12"; \
	echo "RUN demo_weak"; \
	test "$$(./demo_weak)" = "value=42"; \
	echo "RUN demo_ar"; \
	test "$$(./demo_ar)" = "hello, ada"; \
	echo "RUN demo_so"; \
	test "$$(./demo_so)" = "hello, ada"; \
	echo "RUN scripts/run_so.sh"; \
	test "$$(bash scripts/run_so.sh)" = "hello, ada"; \
	echo "RUN demo_dlopen"; \
	test "$$(./demo_dlopen)" = "hello, dynamic"; \
	echo "RUN demo_interpose_compile"; \
	test "$$(./demo_interpose_compile)" = "mallocs=1"; \
	echo "RUN demo_interpose_wrap"; \
	test "$$(./demo_interpose_wrap)" = "wraps=1"; \
	echo "RUN scripts/run_preload.sh"; \
	out=$$(bash scripts/run_preload.sh 2>&1); \
	printf '%s\n' "$$out" | grep -qx 'target-ok'; \
	printf '%s\n' "$$out" | grep -qx 'preload-mallocs=3'; \
	echo "demos-run ok"

test-solution: test_solution_bin demos-run
	./test_solution_bin

clean:
	rm -rf $(BUILD)
	rm -f demo_static_multi demo_weak demo_ar demo_so demo_dlopen \
		demo_interpose_compile demo_interpose_wrap demo_preload_target \
		libscgreet.a libscgreet.so libpreload.so \
		test_student test_solution_bin
