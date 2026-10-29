#!/usr/bin/env python3
"""Check positioned axes stay within the figure and leave a title gap."""
import ast
import pathlib
import subprocess
import sys

executor, source, output = sys.argv[1:]
subprocess.run([executor, f"--output={output}", source], check=True)
text = pathlib.Path(output).read_text(encoding="utf-8")
positions = []
for line in text.splitlines():
    if line.startswith("ax = fig.add_axes("):
        positions.append(ast.literal_eval(line.split("fig.add_axes(", 1)[1].rsplit(")", 1)[0]))
assert len(positions) == 2, positions
for x, y, w, h in positions:
    assert 0.05 < x < 0.9 and 0.05 < y < 0.9
    assert 0 < w < 0.9 and 0 < h < 0.9
    assert x + w < 0.95 and y + h < 0.95
assert positions[0][1] > positions[1][1] + positions[1][3] + 0.03, positions
