"""Check CUB's parametric ellipses mixed with inline observation data."""
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
assert Path(output).is_file()
plt.close("all")
runpy.run_path(output, run_name="__main__")
ax = plt.gcf().axes[0]
# Each parametric pair produces one line; the inline block remains a data series.
assert len(ax.lines) == 2, len(ax.lines)
assert len(ax.collections) == 1, len(ax.collections)
points = ax.collections[0].get_offsets()
assert points.shape == (5, 2), points.shape
assert abs(points[0, 0] - 0.3195757409) < 1e-10
assert abs(points[0, 1] - 0.8051050332) < 1e-10
assert abs(points[-1, 0] - 0.2204504556) < 1e-10
assert abs(points[-1, 1] - 0.8514042370) < 1e-10
plt.close("all")
