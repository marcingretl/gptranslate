#!/usr/bin/env python3
"""Check figure titles, skipped cells and retained panel positions in gridplot."""
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

# Skipping panels must not renumber subsequent plot commands.
assert "ax = axes[0]" in code
assert "ax = axes[2]" in code
assert "ax = axes[5]" in code
assert "fig.suptitle('Gridplot: skipped cells')" in code
assert "fig.tight_layout(rect=(0, 0, 1, 0.94))" in code

namespace = {"__name__": "__main__"}
exec(compile(code, output, "exec"), namespace)
fig = namespace["fig"]
assert len(fig.axes) == 6
assert fig._suptitle.get_text() == "Gridplot: skipped cells"

# Rows-first ordering and skipped cells should survive a full execution.
expected = {0: ("First panel", [10, 20]),
            2: ("Third panel", [30, 40]),
            5: ("Sixth panel", [50, 60])}
for i, ax in enumerate(fig.axes):
    if i in expected:
        title, values = expected[i]
        assert ax.get_visible(), i
        assert ax.get_title() == title, i
        assert len(ax.lines) == 1, i
        assert list(ax.lines[0].get_ydata()) == values, i
    else:
        assert not ax.get_visible(), i

fig.canvas.draw()
plt.close(fig)
