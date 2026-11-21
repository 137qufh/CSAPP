# Stop at main, show the stack, the registers, and a few instructions.
# From chapters/ch03_machine, after `make`:
#   gdb -q -x gdb/walk.gdb ./demo_call
# Pass any other chapter demo the same way. This file does not choose
# the binary; gdb takes it from the command line.

set confirm off
set pagination off
set disassembly-flavor att

break main
run
backtrace
info registers
disassemble main
x/8i $pc
quit
