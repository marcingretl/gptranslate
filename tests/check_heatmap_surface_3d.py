#!/usr/bin/env python3
"""Verify that heatmap pm3d creates a real 3D surface, not a 2D raster."""
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

# The selected plot owns one Axes3D and its accompanying colorbar.
assert axis.name == "3d", axis.name
assert axis in figure.axes
assert len(figure.axes) == 2, len(figure.axes)
assert axis.get_zlabel() == "z", axis.get_zlabel()
assert axis.zaxis.label.get_fontsize() == 12
assert axis.elev == 35 and axis.azim == -40
# Global tic label offset is applied to each 3D axis as a pad approximation.
assert "ax.tick_params(axis='x', pad=" in code
assert "ax.tick_params(axis='y', pad=" in code
assert "ax.tick_params(axis='z', pad=" in code
assert np.allclose(namespace["_gp_surface_z"],
                   [[0, 0.5, 1], [0.5, 1, 1.5], [1, 1.5, 2]])
assert len(axis.collections) == 1
plt.close(figure)
print("heatmap 3D surface runtime: OK")
