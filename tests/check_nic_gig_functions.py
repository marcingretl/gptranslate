#!/usr/bin/env python3
"""Verify genuine nic_gig named expressions, ternaries and reference axes."""
import ast
import math
from pathlib import Path
import subprocess
import sys

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.colors import to_hex

executor, fixture, output, invalid_fixture = sys.argv[1:5]
result = subprocess.run([executor, '--no-show', '--output', output, fixture],
                        capture_output=True, text=True)
assert result.returncode == 0, result.stderr
source = Path(output).read_text(encoding='utf-8')
ast.parse(source)
assert 'plt.show()' not in source
assert 'if ((value<0)) else' not in source  # Conditions must remain valid Python.

namespace = {'__name__': '__main__'}
exec(compile(source, output, 'exec'), namespace)
fig = namespace['fig']
ax = fig.axes[0]
assert ax.get_xlim() == (-3.0, 3.0)
assert ax.get_ylim() == (-0.1, 6.0)
assert ax.get_xlabel() == 'Standardised Residual (e_{t-1})'
assert ax.get_ylabel() == 'Conditional Variance h_t'

# Two analytic NIC curves plus the styled zero reference axes.
lines = {line.get_label(): line for line in ax.lines}
assert 'GARCH(1,1)' in lines and 'GJR-GARCH(1,1)' in lines

garch = lines['GARCH(1,1)']
gjr = lines['GJR-GARCH(1,1)']
x1, y1 = list(map(float, garch.get_xdata())), list(map(float, garch.get_ydata()))
x2, y2 = list(map(float, gjr.get_xdata())), list(map(float, gjr.get_ydata()))
assert len(x1) == len(y1) == len(x2) == len(y2) == 200
assert x1 == x2
for x, a, b in zip(x1, y1, y2):
    assert math.isfinite(a) and math.isfinite(b)
    assert abs(a - (0.00041496 + 0.111398 * x**2)) < 1e-10
    expected = 0.000276527 + (0.562529 if x < 0 else 0.0857941) * x**2
    assert abs(b - expected) < 1e-10
assert to_hex(garch.get_color()) == '#0072b2'
assert to_hex(gjr.get_color()) == '#d55e00'

axis_lines = [line for line in ax.lines if line not in (garch, gjr)]
assert len(axis_lines) == 2, f'Expected both reference axes, found {len(axis_lines)}'
hline, vline = axis_lines
assert list(hline.get_ydata()) == [0, 0]
assert list(vline.get_xdata()) == [0, 0]
assert to_hex(hline.get_color()) == '#808080'
assert to_hex(vline.get_color()) == '#447799'
assert abs(hline.get_linewidth() - 1.0) < 1e-10
assert abs(vline.get_linewidth() - 1.25) < 1e-10

# Security regression: arbitrary Python calls in gnuplot function bodies
# must not be copied into the generated program.
rejected = subprocess.run([executor, '--no-show', '--output', output, invalid_fixture],
                          capture_output=True, text=True)
assert rejected.returncode != 0
assert 'unsupported named analytic function' in rejected.stderr
plt.close('all')
print('nic_gig named and piecewise functions: OK')
