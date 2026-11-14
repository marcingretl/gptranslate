#!/usr/bin/env python3
"""Exercise BACE's grouped Prior/PIP histogram with a headless backend."""
import os
import runpy
import subprocess
import sys

os.environ["MPLBACKEND"] = "Agg"
import matplotlib.pyplot as plt

executor, source, output = sys.argv[1:4]
result = subprocess.run([executor, f"--output={output}", source],
                        text=True, capture_output=True)
assert result.returncode == 0, result.stderr
ns = runpy.run_path(output)
fig = ns["fig"]
ax = fig.axes[0]
assert [text.get_text() for text in ax.get_xticklabels()] == ["1", "2", "3"]
assert sorted(round(bar.get_height(), 2) for bar in ax.patches) == [
    0.25, 0.35, 0.4, 0.45, 0.65, 0.9
]
assert len({round(bar.get_x(), 3) for bar in ax.patches}) == 6
assert [label.get_text() for label in ax.get_legend().get_texts()] == ["Prior", "PIP"]
# A `below` legend is below the chart, not the default right-hand side.
assert ax.get_legend().get_bbox_to_anchor()._bbox.y0 < 0
plt.close(fig)
