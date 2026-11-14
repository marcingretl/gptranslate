"""Verify the XYZ map interpolation and the correspondence-analysis box ratio."""
import pathlib
import subprocess
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

executor, source, target, kind = sys.argv[1:]
result = subprocess.run([executor, "--no-show", f"--output={target}", source],
                        capture_output=True, text=True)
assert result.returncode == 0, result.stderr
namespace = {"__name__": "__main__"}
script = pathlib.Path(target)
exec(compile(script.read_text(encoding="utf-8"), str(script), "exec"), namespace)
figure, axis = namespace["fig"], namespace["ax"]
figure.canvas.draw()

if kind in ("contour", "pm3d"):
    assert axis.name == "rectilinear", "a flat map must not create a 3D axes"
    grid_x, grid_y = namespace["_gp_map_x"], namespace["_gp_map_y"]
    grid_z = namespace["_gp_contour_values"]
    assert grid_x.shape == grid_y.shape == grid_z.shape == (21, 21)
    assert np.isclose(grid_x.min(), 0) and np.isclose(grid_x.max(), 2)
    assert np.isclose(grid_y.min(), 0) and np.isclose(grid_y.max(), 2)
    assert np.isclose(grid_z[10, 10], 0.37), "exact XYZ hit was not preserved"
    assert np.isfinite(grid_z).all()
    if kind == "contour":
        assert len(figure.axes) == 1
        assert np.allclose(namespace["_gp_contour"].levels, [0.10, 0.20, 0.30])
        assert len(axis.collections) >= 1
    else:
        assert len(figure.axes) == 2, "pm3d map should generate its colorbar"
        assert axis.collections, "pm3d map must produce a shaded mesh"
        assert not axis.images, "pm3d must use the actual XYZ coordinate grid"
else:
    assert kind == "ratio"
    assert abs(axis.get_box_aspect() - 0.692225) < 1e-8

plt.close(figure)
print(kind, "stage2 rendering: OK")
