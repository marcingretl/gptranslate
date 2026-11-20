#!/usr/bin/env python3
"""Check that outside-below legends do not cover x-axis annotations."""
from pathlib import Path
import subprocess
import sys
import tempfile

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

executor, fixture, output = sys.argv[1:4]
source = Path(fixture).read_text(encoding="utf-8")

# Use the real Threshold_Panel style loop and add its title and x-axis label.
source = source.replace("set key outside below font", "set xlabel 'Threshold Parameter no. 1'\nset key outside below font", 1)

with tempfile.TemporaryDirectory() as tmp:
    gpfile = Path(tmp) / "threshold-panel-key.gp"
    gpfile.write_text(source, encoding="utf-8")
    result = subprocess.run([executor, "--no-show", "--output", output, str(gpfile)],
                            text=True, capture_output=True)
    assert result.returncode == 0, result.stderr
    namespace = {"__name__": "__main__"}
    exec(compile(Path(output).read_text(encoding="utf-8"), output, "exec"), namespace)
    fig = namespace["fig"]
    ax = fig.axes[0]
    fig.canvas.draw()
    renderer = fig.canvas.get_renderer()
    key = ax.get_legend()
    assert key is not None
    keybox = key.get_window_extent(renderer)
    labelbox = ax.xaxis.label.get_window_extent(renderer)
    ticks = [tick.get_window_extent(renderer) for tick in ax.get_xticklabels() if tick.get_visible()]
    assert keybox.y1 < labelbox.y0 - 3, (keybox.bounds, labelbox.bounds)
    assert all(keybox.y1 < tick.y0 - 3 for tick in ticks), keybox.bounds
    assert keybox.y0 >= -1, keybox.bounds
    plt.close(fig)

print("outside-below key placement: OK")
