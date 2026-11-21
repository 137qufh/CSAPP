# Systems Craft — top-level build.
# `make` / `make test` / `make test-solution` all run the reference solutions.
# Inside a chapter, `make test` builds your stubs and `make test-solution` the reference.

CHAPTERS = ch01_tour ch02_data ch03_machine ch04_arch ch05_opt ch06_memory \
	ch07_linking ch08_ecf ch09_vm ch10_io ch11_network ch12_concurrent
LABS = bitcraft asmcraft cachecraft shellcraft allocraft proxycraft

.PHONY: all test test-solution test-student clean \
	$(CHAPTERS) $(addprefix lab-,$(LABS)) capstone common

all: common
	@set -e; for c in $(CHAPTERS); do $(MAKE) -C chapters/$$c; done
	@set -e; for l in $(LABS); do $(MAKE) -C labs/$$l; done
	$(MAKE) -C capstone

test: test-solution

test-solution: common
	$(MAKE) -C common test
	@set -e; for c in $(CHAPTERS); do $(MAKE) -C chapters/$$c test-solution; done
	@set -e; for l in $(LABS); do $(MAKE) -C labs/$$l test-solution; done
	$(MAKE) -C capstone test-solution

test-student:
	@set -e; for c in $(CHAPTERS); do $(MAKE) -C chapters/$$c test; done
	@set -e; for l in $(LABS); do $(MAKE) -C labs/$$l test; done

clean:
	$(MAKE) -C common clean
	@for c in $(CHAPTERS); do $(MAKE) -C chapters/$$c clean; done
	@for l in $(LABS); do $(MAKE) -C labs/$$l clean; done
	$(MAKE) -C capstone clean

common:
	$(MAKE) -C common

capstone:
	$(MAKE) -C capstone

ch01 ch01_tour:
	$(MAKE) -C chapters/ch01_tour
ch02 ch02_data:
	$(MAKE) -C chapters/ch02_data
ch03 ch03_machine:
	$(MAKE) -C chapters/ch03_machine
ch04 ch04_arch:
	$(MAKE) -C chapters/ch04_arch
ch05 ch05_opt:
	$(MAKE) -C chapters/ch05_opt
ch06 ch06_memory:
	$(MAKE) -C chapters/ch06_memory
ch07 ch07_linking:
	$(MAKE) -C chapters/ch07_linking
ch08 ch08_ecf:
	$(MAKE) -C chapters/ch08_ecf
ch09 ch09_vm:
	$(MAKE) -C chapters/ch09_vm
ch10 ch10_io:
	$(MAKE) -C chapters/ch10_io
ch11 ch11_network:
	$(MAKE) -C chapters/ch11_network
ch12 ch12_concurrent:
	$(MAKE) -C chapters/ch12_concurrent

lab-bitcraft:
	$(MAKE) -C labs/bitcraft
lab-asmcraft:
	$(MAKE) -C labs/asmcraft
lab-cachecraft:
	$(MAKE) -C labs/cachecraft
lab-shellcraft:
	$(MAKE) -C labs/shellcraft
lab-allocraft:
	$(MAKE) -C labs/allocraft
lab-proxycraft:
	$(MAKE) -C labs/proxycraft
