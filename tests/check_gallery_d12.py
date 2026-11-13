#!/usr/bin/env python3
"""Regression test for gretl4py Dataset.scatters() and graph-relative offsets."""
import pathlib
import subprocess
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

executor, source, output = sys.argv[1:4]
subprocess.run([executor, "--no-show", "--output", output, source], check=True)
code = pathlib.Path(output).read_text(encoding="utf-8")
compile(code, output, "exec")

namespace = {"__name__": "__main__"}
exec(compile(code, output, "exec"), namespace)
fig = namespace["fig"]
assert len(fig.axes) == 2
for ax, xlabel in zip(fig.axes, ("demo_signal", "demo_positive")):
    assert ax.get_xlabel() == xlabel
    assert ax.get_ylabel() == "demo_wave"
    # A scatter plot must not join successive observations with lines.
    assert len(ax.collections) == 1
    assert len(ax.collections[0].get_offsets()) == 55
    assert len(ax.lines) == 1  # the zero-reference axis only
    xmin, xmax = ax.dataLim.intervalx
    ymin, ymax = ax.dataLim.intervaly
    x0, x1 = ax.get_xlim()
    y0, y1 = ax.get_ylim()
    # Four 0.02 graph-relative offsets add 2% of the final plot span per side.
    assert abs((xmin - x0) / (x1 - x0) - 0.02) < 1e-8
    assert abs((x1 - xmax) / (x1 - x0) - 0.02) < 1e-8
    assert abs((ymin - y0) / (y1 - y0) - 0.02) < 1e-8
    assert abs((y1 - ymax) / (y1 - y0) - 0.02) < 1e-8
fig.canvas.draw()
plt.close(fig)
