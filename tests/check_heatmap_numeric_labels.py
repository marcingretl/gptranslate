#!/usr/bin/env python3
"""Check that plain `matrix using ... sprintf(...) with labels` renders."""
import ast
from pathlib import Path
import subprocess
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

executor, source, output = sys.argv[1:]
result = subprocess.run([executor, f"--output={output}", source],
                        capture_output=True, text=True)
assert result.returncode == 0, result.stderr
code = Path(output).read_text(encoding="utf-8")
ast.parse(code)
namespace = {"__name__": "__main__"}
exec(compile(code, output, "exec"), namespace)
ax = namespace["ax"]
assert len(ax.images) == 1
labels = {(round(t.get_position()[0]), round(t.get_position()[1])): t
          for t in ax.texts}
assert len(labels) == 4, labels
expected = {(0, 0): "0.1", (1, 0): "0.5", (0, 1): "-0.8", (1, 1): "1.0"}
assert {xy: t.get_text() for xy, t in labels.items()} == expected
assert all(t.get_fontsize() == 12 for t in labels.values())
assert all(t.get_color() == "black" for t in labels.values())
plt.close(namespace["fig"])
print("heatmap numeric matrix labels: OK")
