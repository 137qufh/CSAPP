#!/usr/bin/env bash
# Inspect the ELF image produced from demo_hello.c.
# Requires binutils (readelf, objdump) which ship with build-essential.
set -euo pipefail
cd "$(dirname "$0")/.."

if [[ ! -x demo_hello ]]; then
    gcc -Wall -Wextra -std=gnu11 -O1 -g demo_hello.c -o demo_hello
fi

echo "== file =="
file demo_hello
echo
echo "== ELF header =="
readelf -h demo_hello
echo
echo "== section headers =="
readelf -S demo_hello
echo
echo "== symbol sample =="
nm -n demo_hello | head -n 30
echo
echo "== disassemble main =="
objdump -d --no-show-raw-insn demo_hello | awk '
    /^[0-9a-f]+ <main>:/ {p=1}
    p {print}
    p && /^$/ {exit}
'
