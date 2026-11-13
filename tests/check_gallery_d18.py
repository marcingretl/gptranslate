#!/usr/bin/env python3
"""Regression test for triangular correlation heatmaps with missing cells."""
import pathlib
import subprocess
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

executor, source, output = sys.argv[1:4]
subprocess.run([executor, "--no-show", "--output", output, source], check=True)
code = pathlib.Path(output).read_text(encoding="utf-8")
compile(code, output, "exec")

namespace = {"__name__": "__main__"}
exec(compile(code, output, "exec"), namespace)
fig = namespace["fig"]
ax = fig.axes[0]
assert len(ax.images) == 1
values = np.ma.asarray(ax.images[0].get_array())
assert values.shape == (4, 4)
assert np.isnan(np.asarray(values)).sum() == 6
assert abs(float(values[0, 0]) + 0.6592) < 1e-9
assert abs(float(values[1, 0]) + 0.8066) < 1e-9
assert abs(float(values[3, 0]) - 1.0) < 1e-9
assert len(ax.texts) == 10  # no labels for NaN cells
assert [t.get_text() for t in ax.texts[:4]] == ["-0.7", "-0.5", "0.8", "1.0"]
assert [t.get_text() for t in ax.texts[-1:]] == ["1.0"]
assert ax.get_title() == "Correlation matrix"
fig.canvas.draw()
plt.close(fig)
