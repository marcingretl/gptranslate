#!/usr/bin/env python3
"""Verify that the SVAR historical-decomposition script keeps stacked histograms."""
import pathlib
import subprocess
import sys

exe, source, output = sys.argv[1:]
subprocess.run([exe, "--emit-python", "--output", output, source], check=True)
# The translator writes next to the source when invoked with --emit-python.
code = pathlib.Path(output).read_text()
assert code.count("target.bar(x, y, bottom=_hist_bottom") == 2
assert "_hist_positive = {}" in code
assert "_hist_negative = {}" in code
assert "target.plot(x, y, zorder=3" in code
assert "alpha=0.5" in code
compile(code, output, "exec")

# Verify that the three series read distinct data columns from the named block.
# In particular, the abbreviated 'u 3' must not fall back to column 2.
import matplotlib
matplotlib.use("Agg")

namespace = {"__name__": "__main__"}
exec(compile(code, output, "exec"), namespace)
fig = namespace["fig"]
ax = fig.axes[0]
assert len(ax.containers) == 2
assert len(ax.containers[0]) == 160
assert len(ax.containers[1]) == 160
assert ax.containers[0].get_label() == "unemp"
assert ax.containers[1].get_label() == "infl"
assert abs(ax.containers[0][0].get_height() - 0.29133) < 1e-10  # column 2
assert abs(ax.containers[1][0].get_height() - (-1.0654)) < 1e-10  # column 3
line = next(line for line in ax.lines if line.get_label() == "infl (stoch. component)")
assert len(line.get_xdata()) == 160
assert list(line.get_xdata())[:2] == [1, 2]
assert abs(line.get_ydata()[0] - (-0.77402)) < 1e-10  # column 1
fig.canvas.draw()
