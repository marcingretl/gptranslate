#!/usr/bin/env python3
"""Check named line styles with optional linewidth (naiveFC regression)."""
from pathlib import Path
import subprocess
import sys

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.colors import to_hex


exe, source, output = sys.argv[1:4]
subprocess.run([exe, '--no-show', '--output', output, source], check=True)
code = Path(output).read_text(encoding='utf-8')
namespace = {'__name__': '__main__'}
exec(compile(code, output, 'exec'), namespace)
fig = namespace['fig']
ax = fig.axes[0]
fig.canvas.draw()

# The definition without 'lw' must retain the backend's default width.
assert len(ax.lines) == 2
assert to_hex(ax.lines[0].get_color()) == '#808080'
assert ax.lines[0].get_linewidth() == 1.5
assert ax.lines[0].get_linestyle() == '-'

# Existing explicit widths and special lt=0 handling must not regress.
assert to_hex(ax.lines[1].get_color()) == '#cc0000'
assert ax.lines[1].get_linewidth() == 2
assert ax.lines[1].get_linestyle() == ':'
plt.close(fig)
print('naiveFC named line style without lw: OK')
