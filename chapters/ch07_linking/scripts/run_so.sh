#!/usr/bin/env bash
# demo_so records DT_RUNPATH=$ORIGIN, so the loader looks for
# libscgreet.so beside the binary. LD_LIBRARY_PATH is the other
# usual knob; this runner sets it as well.
set -euo pipefail
cd "$(dirname "$0")/.."

if [[ ! -x demo_so || ! -f libscgreet.so ]]; then
    echo "build first: make -C chapters/ch07_linking" >&2
    exit 1
fi

export LD_LIBRARY_PATH="$PWD${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
exec ./demo_so
