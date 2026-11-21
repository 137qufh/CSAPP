#!/usr/bin/env python3
"""Read memory_mountain.csv and, when matplotlib is present, draw a PNG."""

import csv
import os
import sys


def main(argv):
    try:
        import matplotlib

        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except ImportError:
        print("matplotlib not installed, CSV only")
        return 0

    path = argv[1] if len(argv) > 1 else "memory_mountain.csv"
    if not os.path.isfile(path):
        here = os.path.dirname(os.path.abspath(__file__))
        alt = os.path.join(os.path.dirname(here), "memory_mountain.csv")
        if os.path.isfile(alt):
            path = alt
        else:
            print("missing %s" % path, file=sys.stderr)
            return 1

    series = {}
    with open(path, newline="") as handle:
        reader = csv.DictReader(handle)
        for row in reader:
            stride = int(row["stride"])
            size = int(row["size_bytes"])
            ns = float(row["ns_per_access"])
            series.setdefault(stride, []).append((size, ns))

    figure, axis = plt.subplots(figsize=(7, 4))
    for stride in sorted(series):
        points = sorted(series[stride])
        axis.plot(
            [p[0] for p in points],
            [p[1] for p in points],
            marker="o",
            label="stride %d" % stride,
        )
    axis.set_xscale("log", base=2)
    axis.set_xlabel("working set (bytes)")
    axis.set_ylabel("ns per access")
    axis.set_title("memory mountain")
    axis.legend()
    figure.tight_layout()

    out_dir = os.path.dirname(os.path.abspath(path))
    out = os.path.join(out_dir, "memory_mountain.png")
    figure.savefig(out)
    print(out)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
