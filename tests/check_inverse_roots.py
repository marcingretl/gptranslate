#!/usr/bin/env python3
"""Test polar root geometry and general axis settings using real gretl syntax."""
import math
import pathlib
import runpy
import subprocess
import sys

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

executor, source, output = sys.argv[1:4]
subprocess.run([executor, '--no-show', '--output', output, source], check=True)
code = pathlib.Path(output).read_text(encoding='utf-8')
assert 'plt.show()' not in code, 'translation test must not display a figure'
runpy.run_path(output, run_name='__main__')
fig = plt.gcf()
assert len(fig.axes) == 1
ax = fig.axes[0]

# The constant polar function r=1 must create a closed unit circle, not y=1.
assert len(ax.lines) == 3, 'expected unit circle and both zero reference axes'
circle = ax.lines[0]
coords = list(zip(circle.get_xdata(), circle.get_ydata()))
assert len(coords) >= 200
assert all(abs(math.hypot(x, y) - 1.0) < 1e-10 for x, y in coords)
assert abs(coords[0][0] - coords[-1][0]) < 1e-10
assert abs(coords[0][1] - coords[-1][1]) < 1e-10

# The four inline polar pairs are transformed to the original Cartesian roots.
assert len(ax.collections) == 1
points = ax.collections[0].get_offsets()
expected = [(0.5, 0.5), (-0.7, 0.0), (0.2, -0.3), (1.0, 0.0)]
assert len(points) == len(expected)
for actual, target in zip(points, expected):
    assert abs(actual[0] - target[0]) < 1e-7, (actual, target)
    assert abs(actual[1] - target[1]) < 1e-7, (actual, target)

assert not any(spine.get_visible() for spine in ax.spines.values())
assert not ax.get_xticks().size and not ax.get_yticks().size
assert all(abs(x) < 1e-12 for x in ax.lines[2].get_xdata())
assert all(abs(y) < 1e-12 for y in ax.lines[1].get_ydata())

# Square axes and equal units preserve the unit circle's aspect on the canvas.
fig.canvas.draw()
window = ax.get_window_extent()
assert abs(window.width - window.height) < 0.01, (window.width, window.height)
assert abs(ax.get_data_ratio() - 1.0) < 0.01, ax.get_data_ratio()
plt.close('all')

# Both forms of tic suppression are reversible; the latter `set ytics` wins.
extra = pathlib.Path(output).with_name('inverse-roots-tics.gp')
extra.write_text('set noytics\nset ytics 0.25\nset noxtics\nset xtics 0.5\n'
                 'set yzeroaxis\nunset yzeroaxis\nset polar\nunset polar\n'
                 'plot 1 notitle w lines\n', encoding='utf-8')
extra_output = pathlib.Path(output).with_name('inverse-roots-tics.py')
subprocess.run([executor, '--no-show', '--output', str(extra_output), str(extra)], check=True)
runpy.run_path(str(extra_output), run_name='__main__')
ax = plt.gca()
assert len(ax.lines) == 1 and ax.lines[0].get_ydata()[0] == 1.0
assert ax.yaxis.get_major_locator().__class__.__name__ == 'MultipleLocator'
assert ax.xaxis.get_major_locator().__class__.__name__ == 'MultipleLocator'
plt.close('all')
