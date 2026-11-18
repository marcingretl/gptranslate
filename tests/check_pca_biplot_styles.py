#!/usr/bin/env python3
"""Check pcaTools biplot annotations and colored secondary-axis scales."""
import ast
from pathlib import Path
import subprocess
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.colors import to_rgba
from matplotlib.patches import FancyArrowPatch


executor, source, output = sys.argv[1:4]
result = subprocess.run([executor, "--no-show", "--output", output, source],
                        capture_output=True, text=True)
assert result.returncode == 0, result.stderr
code = Path(output).read_text(encoding="utf-8")
ast.parse(code)
namespace = {"__name__": "__main__"}
exec(compile(code, output, "exec"), namespace)
figure = namespace["fig"]
figure.canvas.draw()
axis = namespace["ax"]
blue = to_rgba("#0080ff")

# The two loading arrows retain data coordinates, width and gnuplot RGB color.
arrows = [patch for patch in axis.patches if isinstance(patch, FancyArrowPatch)]
assert len(arrows) == 2, arrows
for arrow, endpoint, width in zip(arrows,
                                  [(-2.91107, 0.308046), (-2.71553, -1.06578)],
                                  [2, 1]):
    assert tuple(arrow._posA_posB[0]) == (0, 0)
    assert all(abs(actual - wanted) < 1e-7
               for actual, wanted in zip(arrow._posA_posB[1], endpoint))
    assert abs(arrow.get_linewidth() - width) < 1e-9
    assert arrow.get_edgecolor() == blue

# Text color/size are independent of arrow styling and tic modifiers.
labels = {label.get_text(): label for label in axis.texts}
assert set(labels) == {"loading a", "loading b"}
for name, fontsize in [("loading a", 12), ("loading b", 10)]:
    assert labels[name].get_color() == "#0080ff"
    assert labels[name].get_fontsize() == fontsize

# Both loading scales have their own [-1, 1] ranges and colored tick labels.
top, right = namespace["top_ax"], namespace["right_ax"]
assert all(abs(a - b) < 1e-9 for a, b in zip(top.get_xlim(), (-1, 1)))
assert all(abs(a - b) < 1e-9 for a, b in zip(right.get_ylim(), (-1, 1)))
for labels in (top.get_xticklabels(), right.get_yticklabels()):
    assert labels
    assert all(to_rgba(label.get_color()) == blue for label in labels)
    assert all(abs(label.get_fontsize() - 12) < 1e-9 for label in labels)
plt.close(figure)
print("pcaTools biplot styles: OK")
