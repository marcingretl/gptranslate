#!/usr/bin/env python3
"""Render a gretl screeplot with line-style and rectangle decorations."""
from pathlib import Path
import subprocess
import sys
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.colors import to_hex

executor, fixture, output = sys.argv[1:4]
result = subprocess.run([executor, '--no-show', '--output', output, fixture],
                        capture_output=True, text=True)
assert result.returncode == 0, result.stderr
namespace = {'__name__': '__main__'}
exec(compile(Path(output).read_text(encoding='utf-8'), output, 'exec'), namespace)
fig = namespace['fig']
ax = fig.axes[0]
fig.canvas.draw()
assert len(ax.lines) == 2
first, second = ax.lines
assert to_hex(first.get_color()) == '#343434'
assert to_hex(second.get_color()) == '#808080'
assert abs(first.get_linewidth() - 1) < 1e-10
assert abs(second.get_linewidth() - 0.6) < 1e-10
assert first.get_marker() == 'o', 'linespoint must plot points as well as lines'
assert second.get_linestyle() == '--', 'dt 2 must produce a dashed style'
assert to_hex(fig.get_facecolor()) == '#c2c2c2', 'screen background was lost'
assert to_hex(ax.get_facecolor()) == '#f5f5f5', 'graph background was lost'
plt.close(fig)
print('DFM screeplot style and background: OK')
