#!/usr/bin/env bash
# Report the tools a chapter needs. Missing tools are warnings, not a hard stop.
set -u
ok=1
for cmd in gcc make gdb objdump readelf python3; do
    if command -v "$cmd" >/dev/null 2>&1; then
        printf 'ok   %s\n' "$cmd"
    else
        printf 'miss %s\n' "$cmd"
        ok=0
    fi
done
gcc -dumpmachine 2>/dev/null || true
exit $((1 - ok))
