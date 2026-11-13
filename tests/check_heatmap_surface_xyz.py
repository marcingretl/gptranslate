#!/usr/bin/env python3
"""Check explicit XYZ pm3d with qnorm gridding and palette coloring."""
import ast
from pathlib import Path
import subprocess
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

executor, source, output = sys.argv[1:4]
result = subprocess.run([executor, "--no-show", "--output", output, source],
                        capture_output=True, text=True)
assert result.returncode == 0, result.stderr
code = Path(output).read_text(encoding="utf-8")
ast.parse(code)
namespace = {"__name__": "__main__"}
exec(compile(code, output, "exec"), namespace)
figure = namespace["fig"]
axis = namespace["ax"]
figure.canvas.draw()

# The XYZ values are selected from data columns, not matrix indices.
xyz = namespace["_gp_xyz"]
assert xyz.shape == (5, 3), xyz.shape
assert np.array_equal(xyz[2], [0, 2, 4])

# qnorm 2 uses a 5x5 regular grid with exact values at input positions.
xgrid = namespace["_gp_surface_x"]
ygrid = namespace["_gp_surface_y"]
zgrid = namespace["_gp_surface_z"]
assert xgrid.shape == ygrid.shape == zgrid.shape == (5, 5)
assert np.isclose(zgrid[0, 0], 0)
assert np.isclose(zgrid[0, -1], 2)
assert np.isclose(zgrid[-1, 0], 4)
assert np.isclose(zgrid[-1, -1], 6)
assert np.isclose(zgrid[2, 2], 3)
assert np.isfinite(zgrid).all()

# The surface is a 3D collection, with a separate Matplotlib colorbar.
assert axis.name == "3d"
assert axis.get_zlabel() == "Height"
assert len(axis.collections) == 1
assert len(figure.axes) == 2
plt.close(figure)
print("heatmap XYZ qnorm pm3d runtime: OK")
