#!/usr/bin/env python3
"""Check ROC labels, justification and coordinate systems."""
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
namespace = {"__name__": "__main__"}
exec(compile(code, output, "exec"), namespace)
figure = namespace["fig"]
axes = namespace["ax"]
figure.canvas.draw()
labels = {label.get_text(): label for label in axes.texts}
assert len(labels) == 6, labels

for name, position, alignment, graph_coordinates in [
    ("Area = 0.73", (0.6, 0.35), "left", False),
    ("std. error = 0.05", (0.6, 0.30), "left", False),
    ("95% C.I. = [0.63, 0.83]", (0.6, 0.25), "left", False),
    ("Max correct", (0.1, 0.05), "left", True),
    ("Center", (0.4, 0.85), "center", True),
    ("Right", (0.8, 0.8), "right", False),
]:
    label = labels[name]
    assert all(abs(actual - expected) < 1e-10
               for actual, expected in zip(label.get_position(), position))
    assert label.get_horizontalalignment() == alignment
    assert label.get_transform() is (axes.transAxes if graph_coordinates else axes.transData)
plt.close(figure)
print("ROC data/graph label alignment: OK")
