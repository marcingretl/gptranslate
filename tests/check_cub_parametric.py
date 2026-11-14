"""Check numbered CUB confidence ellipses and signed affine coefficients."""
import ast
from pathlib import Path
import runpy
import subprocess
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

executor, source, output = sys.argv[1:]
result = subprocess.run([executor, "--no-show", f"--output={output}", source],
                        capture_output=True, text=True)
assert result.returncode == 0, result.stderr
code = Path(output).read_text(encoding="utf-8")
ast.parse(code)
plt.close("all")
runpy.run_path(output, run_name="__main__")
ax = plt.gcf().axes[0]
assert len(ax.lines) == 2, len(ax.lines)
first, second = ax.lines
assert len(first.get_xdata()) == len(first.get_ydata()) == 401
assert len(second.get_xdata()) == len(second.get_ydata()) == 401

# The initial point has cos(0)=1 and sin(0)=0, and t is sampled over [0,2*pi].
assert abs(first.get_xdata()[0] - (0.0473731 + 0.319576)) < 1e-10
assert abs(first.get_ydata()[0] - (0.01 + 0.521)) < 1e-10
assert abs(second.get_xdata()[0] - (-0.2 + 0.1)) < 1e-10
assert abs(second.get_ydata()[0] - (0.3 - 0.4)) < 1e-10
# A symbolic 2*pi upper bound must close both ellipse curves.
assert abs(first.get_xdata()[-1] - first.get_xdata()[0]) < 1e-10
assert abs(first.get_ydata()[-1] - first.get_ydata()[0]) < 1e-10

# At t ~ pi/2, the signed sine coefficient determines the ellipse orientation.
assert abs(first.get_xdata()[100] - (0.319576 - 0.00322292)) < 4e-4
assert abs(first.get_ydata()[100] - (0.521 + 0.04)) < 4e-4
assert abs(second.get_xdata()[100] - (0.1 + 0.1)) < 4e-4
assert abs(second.get_ydata()[100] - (-0.4 - 0.15)) < 4e-4
plt.close("all")
