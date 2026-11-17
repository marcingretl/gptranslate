#!/usr/bin/env python3
"""Check random_forest's expression offsets and BoxXY rectangle geometry."""
import pathlib
import subprocess
import sys

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

exe, source, output, kind = sys.argv[1:5]
subprocess.run([exe, '--no-show', '--output', output, source], check=True)
code = pathlib.Path(output).read_text(encoding='utf-8')
namespace = {'__name__': '__main__'}
exec(compile(code, output, 'exec'), namespace)
fig = namespace['fig']
assert len(fig.axes) == 1
ax = fig.axes[0]
fig.canvas.draw()

# Gnuplot's BoxXY columns define independent horizontal and vertical bounds.
if kind == 'random-forest':
    assert len(ax.patches) == 3
    for rect, value, row in zip(ax.patches, (0.2, 0.4, 0.6), range(3)):
        assert abs(rect.get_x() - 0.0) < 1e-10
        assert abs(rect.get_y() - (row - 0.3)) < 1e-10
        assert abs(rect.get_width() - value) < 1e-10
        assert abs(rect.get_height() - 0.6) < 1e-10
        assert abs(rect.get_alpha() - 0.6) < 1e-10
    assert list(ax.get_yticks()) == [0, 1, 2]
    assert [label.get_text() for label in ax.get_yticklabels()] == [
        'Feature_A', 'Feature_B', 'Feature_C'
    ]
    # 0.5-bw/2 = 0.2 extends the upper automatic bound; lower 0 is fixed.
    assert abs(ax.get_ylim()[0] - 0.0) < 1e-10
    assert abs(ax.get_ylim()[1] - 2.5) < 1e-10
    assert abs(ax.get_xlim()[1] - 0.65) < 1e-10
elif kind == 'bounds':
    assert len(ax.patches) == 2
    for rect, bounds in zip(ax.patches, ((1, 6, 4, 4), (6, 3, 2, 2))):
        assert all(abs(a - b) < 1e-10 for a, b in zip(
            (rect.get_x(), rect.get_y(), rect.get_width(), rect.get_height()),
            bounds
        ))
        assert abs(rect.get_alpha() - 0.75) < 1e-10
else:
    raise ValueError(kind)
plt.close(fig)
