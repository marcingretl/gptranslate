#!/usr/bin/env python3
"""Verify support for PointPlot-generated gnuplot presentation directives."""
import pathlib
import subprocess
import sys

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.colors import to_hex

exe, source, output = sys.argv[1:4]
subprocess.run([exe, '--no-show', '--output', output, source], check=True)
code = pathlib.Path(output).read_text(encoding='utf-8')
namespace = {'__name__': '__main__'}
exec(compile(code, output, 'exec'), namespace)
fig = namespace['fig']
assert len(fig.axes) == 1
ax = fig.axes[0]
fig.canvas.draw()

# Numeric offsets use additive data units, not a percentage of the range.
assert all(abs(a-b) < 1e-10 for a,b in zip(ax.get_xlim(), (-0.02, 1.02)))
assert all(abs(a-b) < 1e-10 for a,b in zip(ax.get_ylim(), (-0.02, 1.02)))
assert all(abs(lbl.get_fontsize() - 6) < 1e-10 for lbl in ax.get_xticklabels())
assert all(abs(lbl.get_fontsize() - 7) < 1e-10 for lbl in ax.get_yticklabels())

# Global pointsize affects the area of the scatter collection.
assert len(ax.collections) == 1
assert abs(ax.collections[0].get_sizes()[0] - 18 * (1.1547 ** 2)) < 1e-8

# The defined line style is resolved by 'ls 12'.
assert len(ax.lines) == 2
assert to_hex(ax.lines[0].get_color()) == '#808080'
assert ax.lines[0].get_linewidth() == 1
assert ax.lines[0].get_linestyle() == ':'
assert to_hex(ax.lines[1].get_color()) == '#cc0000'
assert ax.lines[1].get_marker() == 'o'
assert ax.lines[1].get_markersize() == 16
plt.close(fig)
