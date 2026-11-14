"""Regression coverage for plotting commands used by contributed packages.

The check translates gnuplot input and executes only the emitted Python under
Agg. It does not require gretl or an interactive display.
"""
import pathlib
import runpy
import subprocess
import sys

import matplotlib
matplotlib.use("Agg")
from matplotlib.collections import PolyCollection
from matplotlib.patches import Rectangle, Wedge

executor, source, target, kind = sys.argv[1:]
script = pathlib.Path(target)
script.unlink(missing_ok=True)
result = subprocess.run([executor, f"--output={script}", source],
                        capture_output=True, text=True)
assert result.returncode == 0, result.stderr
assert script.exists(), "translator did not create Python output"
namespace = runpy.run_path(str(script))
fig = namespace["fig"]
fig.canvas.draw()

if kind == "pie":
    assert len(fig.axes) == 1
    axes = fig.axes[0]
    sectors = [item for item in axes.patches if isinstance(item, Wedge)]
    assert len(sectors) == 2
    assert abs(sectors[0].theta1) < 1e-12
    assert abs(sectors[0].theta2 - 115.2) < 1e-10
    assert sectors[0].center == (0.5, 0.5)
    assert {label.get_text() for label in axes.texts} == {"Alpha", "32.0%"}
elif kind == "candles":
    assert len(fig.axes) == 1
    axes = fig.axes[0]
    assert len([patch for patch in axes.patches if isinstance(patch, Rectangle)]) == 2
    assert len(axes.collections) >= 2
    colors = [tuple(round(x, 4) for x in patch.get_facecolor()[:3])
              for patch in axes.patches]
    assert colors[0] != colors[1], "palette conditional did not color candles"
elif kind == "candles-volume":
    assert len(fig.axes) == 2
    assert len(fig.axes[0].patches) == 2
    assert len(fig.axes[1].patches) == 2
    assert fig.axes[1].yaxis.get_major_formatter()(1000, 0).strip() == "1 k"
elif kind == "sankey":
    assert len(fig.axes) == 1
    axes = fig.axes[0]
    assert len([patch for patch in axes.patches if isinstance(patch, Rectangle)]) == 2
    assert len([item for item in axes.collections if isinstance(item, PolyCollection)]) == 1
    assert {label.get_text() for label in axes.texts} == {"Row 1", "Col 1"}
    assert axes.patches[0].get_clip_on() is False
    assert 0 < axes.patches[0].get_alpha() < 0.5
else:
    raise AssertionError("unknown test case: " + kind)

matplotlib.pyplot.close(fig)
print(f"{kind}: translation and rendering passed")
