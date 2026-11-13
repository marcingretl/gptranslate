#!/usr/bin/env python3
"""Run flat contour rendering and reject genuine 3D splot variants."""
import ast
from pathlib import Path
import subprocess
import sys
import tempfile

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

executor, source, output = sys.argv[1:4]
text = Path(source).read_text(encoding="utf-8")
result = subprocess.run([executor, "--no-show", "--output", output, source],
                        capture_output=True, text=True)
assert result.returncode == 0, result.stderr
code = Path(output).read_text(encoding="utf-8")
ast.parse(code)
namespace = {"__name__": "__main__"}
exec(compile(code, output, "exec"), namespace)
fig, ax, contour = namespace["fig"], namespace["ax"], namespace["_gp_contour"]
fig.canvas.draw()
assert len(fig.axes) == 1, "flat contour uses one 2D axis"
assert len(contour.levels) == 8, contour.levels
assert len(contour.get_edgecolors()) == 8
assert np.allclose(contour.get_linewidths(), 2)
assert np.allclose(contour.get_edgecolors()[0], (1, 0, 0, 1))
assert ax.get_xlim() == (-0.5, 49.5)
assert not ax.images, "contours should not silently become a raster heatmap"
assert any(len(path.vertices) > 10 for path in contour.get_paths()), "missing level curves"
plt.close(fig)

# Explicit contour levels should be passed unchanged to Matplotlib.
with tempfile.TemporaryDirectory() as temp:
    temp = Path(temp)
    discrete = temp / "discrete.gp"
    discrete.write_text(text.replace(
        "set cntrparam bspline levels 8 firstlinetype 101",
        "set cntrparam levels discrete -0.05, 0, 0.25, 0.5"), encoding="utf-8")
    script = temp / "discrete.py"
    result = subprocess.run([executor, "--no-show", "--output", str(script), str(discrete)],
                            capture_output=True, text=True)
    assert result.returncode == 0, result.stderr
    ns = {"__name__": "__main__"}
    exec(compile(script.read_text(), str(script), "exec"), ns)
    assert np.allclose(ns["_gp_contour"].levels, [-0.05, 0, 0.25, 0.5])
    plt.close(ns["fig"])

    # 3D pm3d must remain an explicit error, never a silently altered 2D plot.
    surface = temp / "surface.gp"
    surface.write_text(text.replace("splot $data matrix with lines",
                                    "splot $data matrix with pm3d"), encoding="utf-8")
    result = subprocess.run([executor, "--no-show", "--output", str(temp / "surface.py"),
                             str(surface)], capture_output=True, text=True)
    assert result.returncode != 0 and "unsupported 3D splot" in result.stderr, result.stderr

print("heatmap flat contour runtime: OK")
