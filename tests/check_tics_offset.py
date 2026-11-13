#!/usr/bin/env python3
"""Check tick text offsets and independent nooffset axis handling."""
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
# x offset was cleared, while y offset overrides global settings.
assert "get_xticklabels()" not in code, code
assert "get_yticklabels()" in code, code
assert "ScaledTranslation(0.6 * -0.5" in code, code
namespace = {"__name__": "__main__"}
exec(compile(code, output, "exec"), namespace)
figure = namespace["fig"]
figure.canvas.draw()
assert namespace["ax"].get_ylabel() == ""
plt.close(figure)
print("tick-label offsets: OK")
