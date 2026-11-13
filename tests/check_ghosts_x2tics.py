#!/usr/bin/env python3
"""Verify the compact x2tics syntax used by ghosts on a real top axis."""
from pathlib import Path
import subprocess
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

executor, source, output = sys.argv[1:4]
subprocess.run([executor, "--no-show", "--output", output, source], check=True)
code = Path(output).read_text(encoding="utf-8")
ns = {"__name__": "__main__"}
exec(compile(code, output, "exec"), ns)
fig = ns["fig"]
fig.canvas.draw()
assert len(fig.axes) == 1 and len(fig.axes[0].child_axes) == 1, "a secondary x axis must be created"
ax = fig.axes[0].child_axes[0]
expected = [0.261799, 0.261799, 0.392699, 0.523599, 0.785398, 1.5708]
actual = list(ax.get_xticks())
assert len(actual) == len(expected) and all(abs(a-b) < 1e-6 for a,b in zip(actual, expected)), actual
assert [t.get_text() for t in ax.get_xticklabels()] == ["24", "24", "16", "12", "8", "4"]
plt.close(fig)
print("ghosts compact x2tics: OK")
