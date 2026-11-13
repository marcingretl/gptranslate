#!/usr/bin/env python3
"""Check multi-line Gnuplot palette instructions and the resulting RGB stops."""
import ast
from pathlib import Path
import subprocess
import sys
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

executor, source, output = sys.argv[1:4]
result = subprocess.run([executor, "--no-show", "--output", output, source],
                        capture_output=True, text=True)
assert result.returncode == 0, result.stderr
code = Path(output).read_text(encoding="utf-8")
ast.parse(code)
ns = {"__name__": "__main__"}
exec(compile(code, output, "exec"), ns)
fig, ax = ns["fig"], ns["ax"]
fig.canvas.draw()
assert len(ax.images) == 1
cmap = ax.images[0].get_cmap()
assert np.allclose(cmap(0.0)[:3], [0, 0, 0.5])
assert np.allclose(cmap(1.0)[:3], [0.5, 0, 0])
assert np.allclose(cmap(0.5)[:3], [0.5, 1, 0.5], atol=0.015)
plt.close(fig)
print("heatmap multi-line RGB palette: OK")
