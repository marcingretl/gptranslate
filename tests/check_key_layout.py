# Regression checks for gnuplot key placement and BVAR panel layout.
import pathlib
import subprocess
import sys

exe, source, target, kind = sys.argv[1:]
subprocess.run([exe, "--output", target, source], check=True,
               capture_output=True, text=True)
script = pathlib.Path(target).read_text()
compile(script, target, "exec")
if kind == "panplot":
    assert "bbox_to_anchor=(1.02, 0.5)" in script
    assert "loc='center left'" in script
    assert "fig.tight_layout()" in script
    assert "ncol=_key_cols" in script
    assert "fig.set_size_inches" in script
elif kind == "bvar":
    assert "plt.subplots" in script or "add_axes" in script
    assert script.count("target.plot(") >= 2
    assert "fig.tight_layout()" in script

# Execute the translated plot headlessly and inspect the actual legend.
# This catches regressions where the generated code parses but the key
# remains a single oversized column.
import os
import runpy
os.environ['MPLBACKEND'] = 'Agg'
import matplotlib.pyplot as plt
plt.show = lambda: None
ns = runpy.run_path(target)
if kind == 'panplot':
    legend = ns['ax'].get_legend()
    assert legend is not None
    assert legend._ncols > 1, 'large outside key must use multiple columns'
    assert len(legend.get_texts()) == 94
elif kind == 'bvar':
    assert len(ns['fig'].axes) >= 2
plt.close('all')
