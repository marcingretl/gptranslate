#!/usr/bin/env python3
"""Regression for correspondence-analysis label font, alignment and color."""
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
figure.canvas.draw()
labels = {text.get_text(): text for text in namespace["ax"].texts}
assert set(labels) == {"Row 1", "Column 1", "Supplement"}, labels
for name, expected_position, expected_size, expected_color in [
    ("Row 1", (0.2, 0.342722), 10, "#ff0000"),
    ("Column 1", (0.4, -0.85727), 12, "#0000ff"),
    ("Supplement", (-0.3, 0.15), None, "#0000ff"),
]:
    label = labels[name]
    assert all(abs(actual - expected) < 1e-9 for actual, expected in zip(label.get_position(), expected_position))
    assert label.get_horizontalalignment() == "center"
    assert label.get_color().lower() == expected_color
    if expected_size is not None:
        assert abs(label.get_fontsize() - expected_size) < 1e-9
plt.close(figure)
print("corresp label font and color: OK")
