"""Verify that a bare gnuplot x2 link retains identity-mapped ticks."""
import os
import pathlib
import subprocess
import sys

os.environ["MPLBACKEND"] = "Agg"
import matplotlib.pyplot as plt

exe, gp, generated = sys.argv[1:4]
proc = subprocess.run([exe, "--no-show", "--output", generated, gp], capture_output=True, text=True)
assert proc.returncode == 0, proc.stderr
# Generated file name is based on the source path for --emit-python.
source_py = pathlib.Path(generated)
assert source_py.is_file(), f"No generated file: {source_py}"
text = source_py.read_text(encoding="utf-8")
assert "secondary_xaxis('top')" in text, text
# Execute translated code under Agg and inspect both axes.
namespace = {"__name__": "__main__"}
exec(compile(text, str(source_py), "exec"), namespace)
fig = namespace["fig"]
fig.canvas.draw()
ax = namespace["ax"]
top = namespace["top_ax"]
assert top.get_xticks().tolist() == [0.5, 1.5, 2.5]
assert [t.get_text() for t in top.get_xticklabels()] == ["A", "B", "C"]
assert tuple(round(v, 6) for v in top.get_xlim()) == tuple(round(v, 6) for v in ax.get_xlim())
plt.close(fig)
source_py.unlink(missing_ok=True)
