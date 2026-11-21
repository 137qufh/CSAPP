#!/usr/bin/env bash
# Build a relocatable object and show the records the linker must fix.
#
# An object file does not know the final address of another file's
# function or variable. The assembler leaves a placeholder and a
# relocation entry (readelf -r). objdump -d shows the unfinished
# instruction bytes those entries describe.
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build

gcc -Wall -Wextra -std=gnu11 -g -O1 -fwrapv -D_GNU_SOURCE \
    -c -o build/reloc_sample.o src/reloc_sample.c

echo "== relocations: readelf -r build/reloc_sample.o =="
readelf -r build/reloc_sample.o
echo
echo "== disassembly: objdump -d build/reloc_sample.o =="
objdump -d build/reloc_sample.o
