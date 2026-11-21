#!/usr/bin/env bash
# Interpose libpreload.so on demo_preload_target.
# The library writes preload-mallocs=N to stderr when the process exits.
set -euo pipefail
cd "$(dirname "$0")/.."

if [[ ! -x demo_preload_target || ! -f libpreload.so ]]; then
    echo "build first: make -C chapters/ch07_linking" >&2
    exit 1
fi

exec env LD_PRELOAD="$PWD/libpreload.so" ./demo_preload_target
