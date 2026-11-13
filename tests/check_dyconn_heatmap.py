#!/usr/bin/env python3
"""Check global gnuplot tics and default heatmap colorbox rendering."""
from pathlib import Path
import subprocess
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

executor, source, output = sys.argv[1:4]
subprocess.run([executor, "--no-show", "--output", output, source], check=True)
code = Path(output).read_text(encoding="utf-8")
namespace = {"__name__": "__main__"}
exec(compile(code, output, "exec"), namespace)
fig = namespace["fig"]
fig.canvas.draw()
assert len(fig.axes) == 2, "set colorbox default must re-enable the colorbar"
ax = fig.axes[0]
assert len(ax.images) == 1
assert all(abs(label.get_fontsize() - 12) < 1e-10 for label in ax.get_xticklabels())
assert all(abs(label.get_fontsize() - 12) < 1e-10 for label in ax.get_yticklabels())
assert ax.xaxis.majorTicks[0]._tickdir == "in"
assert ax.yaxis.majorTicks[0]._tickdir == "in"
assert not ax.xaxis.majorTicks[0].tick2line.get_visible()
assert not ax.yaxis.majorTicks[0].tick2line.get_visible()
plt.close(fig)
print("DYconn heatmap global tics/colorbox: OK")
