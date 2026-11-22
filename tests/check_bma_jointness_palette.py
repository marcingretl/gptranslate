#!/usr/bin/env python3
"""Check BMA jointness missing cells and repeated palette breakpoint handling."""
from pathlib import Path
import subprocess
import sys
import tempfile

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

executor, source, output, case = sys.argv[1:5]
result = subprocess.run([executor, "--no-show", "--output", output, source],
                        capture_output=True, text=True)
assert result.returncode == 0, result.stderr
code = Path(output).read_text(encoding="utf-8")
assert "float('nan')" in code
assert "LinearSegmentedColormap.from_list" in code
namespace = {"__name__": "__main__"}
exec(compile(code, output, "exec"), namespace)
fig, ax = namespace["fig"], namespace["ax"]
fig.canvas.draw()
assert len(ax.images) == 1
image = ax.images[0]
data = np.ma.asarray(image.get_array())
assert data.shape == (3, 3)
assert np.isnan(np.asarray(data)).sum() == 3
assert np.ma.count(data) == 6
cmap = image.get_cmap()
if case == "repeated":
    assert np.allclose(cmap(0.0)[:3], [0, 0, 1])
    assert np.allclose(cmap(1.0)[:3], [1, 0, 0])
else:
    assert case == "flat"
    assert np.allclose(cmap(0.0)[:3], [1, 0, 0])
    assert np.allclose(cmap(0.5)[:3], [1, 0, 0])
plt.close(fig)

# A decreasing sequence of breakpoints remains invalid. Equal adjacent
# breakpoints must not inadvertently disable this validation.
if case == "repeated":
    with tempfile.TemporaryDirectory() as tmp:
        invalid = Path(tmp) / "decreasing.gp"
        invalid.write_text("set palette defined (0 'white', -1 'blue', 1 'red')\n" +
                           "$map << EOD\n0 1\n1 0\nEOD\n" +
                           "plot $map matrix with image\n", encoding="utf-8")
        rejected = subprocess.run([executor, "--no-show", str(invalid)],
                                  capture_output=True, text=True)
        assert rejected.returncode != 0
        assert "palette breakpoints must be nondecreasing" in rejected.stderr

print("BMA jointness palette " + case + ": OK")
