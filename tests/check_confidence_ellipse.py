"""Check libgretl's coefficient confidence ellipse and its marginal intervals."""
import ast
import pathlib
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
code = pathlib.Path(output).read_text(encoding="utf-8")
ast.parse(code)

plt.close("all")
runpy.run_path(output, run_name="__main__")
fig = plt.gcf()
assert len(fig.axes) == 1, len(fig.axes)
ax = fig.axes[0]

# A full confidence ellipse plus four marginal interval boundaries.
assert len(ax.lines) == 6, len(ax.lines)
for line in ax.lines[:5]:
    assert len(line.get_xdata()) == 401
    assert len(line.get_ydata()) == 401

# The point marker is located at the joint coefficient estimate.
marker = ax.lines[5]
assert abs(float(marker.get_xdata()[0]) - 1.5) < 1e-10
assert abs(float(marker.get_ydata()[0]) + 0.6) < 1e-10
assert len(ax.texts) == 1
assert ax.texts[0].get_text() == '1.5, -0.6'

# Both axes and the expected marginal confidence lines are preserved.
assert ax.get_xlabel() == 'LRM'
assert ax.get_ylabel() == 'IBO'
assert abs(float(ax.lines[1].get_xdata()[0]) - 1.2) < 1e-10
assert abs(float(ax.lines[2].get_xdata()[0]) - 1.8) < 1e-10
assert abs(float(ax.lines[3].get_ydata()[0]) + 0.9) < 1e-10
assert abs(float(ax.lines[4].get_ydata()[0]) + 0.3) < 1e-10
plt.close("all")
