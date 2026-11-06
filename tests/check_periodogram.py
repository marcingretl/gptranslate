"""Check both coordinate systems of the residual periodogram's x axes."""
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
assert "top_ax = ax.twiny()" in code
assert "top_ax.set_xlabel('periods')" in code

plt.close("all")
runpy.run_path(output, run_name="__main__")
fig = plt.gcf()
assert len(fig.axes) == 2, len(fig.axes)
bottom, top = fig.axes
assert top.get_xlabel() == "periods"
assert abs(top.get_xlim()[0]) < 1e-9
assert abs(top.get_xlim()[1] - 96) < 1e-9
assert abs(top.get_xticks()[0] - 4) < 1e-9
assert top.get_xticklabels()[0].get_text() == "48.0"
assert top.get_xticklabels()[-1].get_text() == ""
assert len(bottom.lines) >= 1

# x2 = 4*x for the scaled example. For the radians example, the
# x2 coordinate at the same position is scaled by (96 / 3.1416).
left, right = bottom.get_xlim()
top_left, top_right = top.get_xlim()
for fraction in (0.0, 0.25, 0.5, 0.75, 1.0):
    x = left + (right - left) * fraction
    x2 = top_left + (top_right - top_left) * fraction
    x_px = bottom.transData.transform((x, 0))[0]
    x2_px = top.transData.transform((x2, 0))[0]
    assert abs(x_px - x2_px) < 1e-5, (x, x2)

if "radians" in source:
    ticks = bottom.get_xticks()
    assert len(ticks) == 5, ticks
    assert abs(ticks[1] - 3.141592653589793 / 4) < 1e-9
plt.close("all")
