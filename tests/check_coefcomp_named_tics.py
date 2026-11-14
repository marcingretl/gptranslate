#!/usr/bin/env python3
"""Validate explicit ticks following axis modifiers as used by coefcomp."""
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
assert list(ax.get_yticks()) == [3.0, 2.0, 1.0], ax.get_yticks()
assert [t.get_text() for t in ax.get_yticklabels()] == [
    "Baseline", "With controls", "Robust"
]
assert list(ax.get_xticks()) == [0.0, 1.0], ax.get_xticks()
assert [t.get_text() for t in ax.get_xticklabels()] == [
    "Zero", "One (sample), more"
]
assert "direction='out'" in code, "trailing 'out' was lost"
plt.close(figure)
print("coefcomp named tics after modifiers: OK")
