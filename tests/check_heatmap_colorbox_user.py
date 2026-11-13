#!/usr/bin/env python3
"""Verify explicit screen-positioned palette colorbars in 2D and 3D."""
import ast
from pathlib import Path
import subprocess
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

executor, source, output, kind = sys.argv[1:5]
result = subprocess.run([executor, "--no-show", "--output", output, source],
                        capture_output=True, text=True)
assert result.returncode == 0, result.stderr
code = Path(output).read_text(encoding="utf-8")
ast.parse(code)
namespace = {"__name__": "__main__"}
exec(compile(code, output, "exec"), namespace)
fig = namespace["fig"]
fig.canvas.draw()

# A user-positioned colorbox belongs to its own figure-relative axes.
assert len(fig.axes) == 2, len(fig.axes)
ax, colorbar_ax = fig.axes
assert ax.name == ("rectilinear" if kind == "reset" else kind), ax.name
if kind == "reset":
    # The ordinary colorbar must be restored after unset/default.
    assert "_gp_manual_colorbars" not in namespace
    assert not np.allclose(colorbar_ax.get_position().bounds,
                           [0.1, 0.6, 0.02, 0.3], atol=1e-3)
    plt.close(fig)
    print("heatmap default colorbox reset: OK")
    sys.exit(0)
assert colorbar_ax is namespace["_gp_colorbox_ax"]
assert namespace["_gp_colorbar"].ax is colorbar_ax
assert np.allclose(colorbar_ax.get_position().bounds,
                   [0.1, 0.6, 0.02, 0.3], rtol=0, atol=1e-8), \
       colorbar_ax.get_position().bounds
assert colorbar_ax.get_ylabel() == "Intensity"
plt.close(fig)
print(f"heatmap positioned {kind} colorbox: OK")
