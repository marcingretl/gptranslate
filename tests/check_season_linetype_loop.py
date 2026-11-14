#!/usr/bin/env python3
"""Validate bounded gnuplot linetype loops and point-size precedence."""
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
figure, ax = namespace["fig"], namespace["ax"]
figure.canvas.draw()
assert len(ax.collections) == 3, len(ax.collections)
# scatter() receives area = 18 * (global pointsize * effective size)**2.
for collection, expected in zip(ax.collections, [162.0, 364.5, 648.0]):
    assert abs(float(collection.get_sizes()[0]) - expected) < 1e-8, \
        (collection.get_sizes(), expected)
# The size-only assignment must not erase an earlier linetype color.
for collection, expected in zip(ax.collections[:2], [(0.0, 1.0, 0.0), (1.0, 0.0, 0.0)]):
    assert all(abs(float(v) - wanted) < 1e-8
               for v, wanted in zip(collection.get_facecolors()[0][:3], expected))
plt.close(figure)
print("season-linetype-loop: OK")
