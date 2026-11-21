#!/usr/bin/env bash
# Show the compilation pipeline on an original hello program.
#   preprocess -> compile to assembly -> assemble -> link
set -euo pipefail
cd "$(dirname "$0")/.."

SRC=demo_hello.c
echo "== preprocess (gcc -E) =="
gcc -E -P -std=gnu11 "$SRC" -o demo_hello.i
wc -l demo_hello.i

echo "== compile to assembly (gcc -S -Og) =="
gcc -S -Og -std=gnu11 -Wall -Wextra "$SRC" -o demo_hello.s
wc -l demo_hello.s

echo "== assemble (gcc -c) =="
gcc -c -Og -std=gnu11 "$SRC" -o demo_hello.o
file demo_hello.o

echo "== link =="
gcc demo_hello.o -o demo_hello_pipe
./demo_hello_pipe

echo
echo "Artifacts: demo_hello.i  demo_hello.s  demo_hello.o  demo_hello_pipe"
