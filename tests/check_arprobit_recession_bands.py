#!/usr/bin/env python3
"""Check the mixed data-x/graph-y recession rectangles emitted by ARprobit."""
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
expected = [(1957.75, 1958.25), (1960.25, 1961.25), (1970, 1971),
            (1974, 1975.25), (1980, 1980.5), (1981.5, 1983),
            (1990.5, 1991.25), (2001.25, 2002)]
assert len(ax.patches) == len(expected), 'Missing recession rectangles'
assert len(ax.lines) == 1
for patch, (left, right) in zip(ax.patches, expected):
    assert abs(patch.get_x() - left) < 1e-9
    assert abs(patch.get_width() - (right - left)) < 1e-9
    assert abs(patch.get_y()) < 1e-9
    assert abs(patch.get_height() - 1) < 1e-9
    assert abs(patch.get_alpha() - 0.5) < 1e-9
    assert to_hex(patch.get_facecolor()) == '#dddddd'
    assert patch.get_zorder() < ax.lines[0].get_zorder()
    # Check the pixel positions against independent data/graph transforms.
    lower_left = patch.get_transform().transform((0, 0))
    upper_right = patch.get_transform().transform((1, 1))
    x_left = ax.transData.transform((left, 0))[0]
    x_right = ax.transData.transform((right, 0))[0]
    y_bottom = ax.transAxes.transform((0, 0))[1]
    y_top = ax.transAxes.transform((0, 1))[1]
    assert abs(lower_left[0] - x_left) < 1e-6
    assert abs(upper_right[0] - x_right) < 1e-6
    assert abs(lower_left[1] - y_bottom) < 1e-6
    assert abs(upper_right[1] - y_top) < 1e-6
plt.close(fig)
print('ARprobit recession bands: OK')
