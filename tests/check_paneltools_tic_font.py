#!/usr/bin/env python3
"""Regress PanelTools-style gnuplot tic fonts with spaces after commas."""
import ast
from pathlib import Path
import subprocess
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

executor, source, output = sys.argv[1:4]
result = subprocess.run([executor, "--no-show", "--output", output, source],
                        capture_output=True, text=True)
assert result.returncode == 0, result.stderr
code = Path(output).read_text(encoding="utf-8")
ast.parse(code)
assert "tick_params(axis='x', labelsize=8" in code, code
assert "tick_params(axis='y', labelsize=9" in code, code

namespace = {"__name__": "__main__"}
exec(compile(code, output, "exec"), namespace)
figure = namespace["fig"]
figure.canvas.draw()
ax = namespace["ax"]
for axis, expected in (("x", 8), ("y", 9)):
    labels = ax.get_xticklabels() if axis == "x" else ax.get_yticklabels()
    assert labels, f"{axis}: no tick labels"
    assert all(abs(label.get_fontsize() - expected) < 1e-9 for label in labels), (
        axis, [label.get_fontsize() for label in labels])
plt.close(figure)
print("PanelTools tic font spacing: OK")
