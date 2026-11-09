#!/usr/bin/env python3
"""Check real libgretl contingency heat map, cell labels and RGB colorbox."""
import ast
from pathlib import Path
import subprocess
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

executor, source, output = sys.argv[1:]
result = subprocess.run([executor, f"--output={output}", source], capture_output=True, text=True)
assert result.returncode == 0, result.stderr
code = Path(output).read_text(encoding="utf-8")
ast.parse(code)
assert "plt.show()" not in code, "queued scripts must not block display"

# Execute the actual emitted source in a fresh namespace, without GUI.
ns = {"__name__": "__main__"}
exec(compile(code, output, "exec"), ns)
fig = ns["fig"]
assert len(fig.axes) == 2, "image axes and colorbar axes expected"
ax, color_axis = fig.axes
assert color_axis.get_ylabel() == "Share of actual class (%)"
assert ax.get_title() == "Classification accuracy: 65.1%"
assert ax.get_xlabel() == "Predicted" and ax.get_ylabel() == "Actual"
assert ax.get_aspect() == 1.0, "gnuplot set size ratio -1 means equal data units"
assert len(ax.images) == 1
image = ax.images[0]
expected = np.array([[3.7069, 14.4802, 81.8129],
                     [20.8773, 33.8701, 45.2526],
                     [64.2294, 26.9534, 8.8172]])
np.testing.assert_allclose(image.get_array(), expected)
assert image.norm.vmin == 0.0 and image.norm.vmax == 100.0

# Gnuplot's normalized numeric RGB components are not byte-quantized.
cmap = image.get_cmap()
np.testing.assert_allclose(cmap(0)[:3], (1.0, 1.0, 1.0))
np.testing.assert_allclose(cmap(255)[:3], (.475, .745, .627))
np.testing.assert_allclose(cmap(0.5)[:3], (.816, .910, .867), atol=0.002)
labels = {(int(text.get_position()[0]), int(text.get_position()[1])): text.get_text()
          for text in ax.texts}
assert labels == {(0,0):"128", (1,0):"500", (2,0):"2825",
                  (0,1):"376", (1,1):"610", (2,1):"815",
                  (0,2):"896", (1,2):"376", (2,2):"123"}, labels

# Splitting a command chain must not split a semicolon inside a quoted label.
with_semicolon = Path(source).read_text().replace('"128" at 0,0', '"128; kept" at 0,0')
alt_source = Path(output).with_suffix('.semicolon.gp')
alt_output = Path(output).with_suffix('.semicolon.py')
try:
    alt_source.write_text(with_semicolon, encoding="utf-8")
    alt = subprocess.run([executor, f"--output={alt_output}", str(alt_source)],
                         capture_output=True, text=True)
    assert alt.returncode == 0, alt.stderr
    assert "128; kept" in alt_output.read_text(encoding="utf-8")
finally:
    alt_source.unlink(missing_ok=True)
    alt_output.unlink(missing_ok=True)
plt.close(fig)
print('confusion matrix heatmap: OK')
