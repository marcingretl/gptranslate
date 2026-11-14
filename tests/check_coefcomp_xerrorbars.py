#!/usr/bin/env python3
"""Verify coefcomp horizontal error bars and existing vertical error bars."""
import pathlib
import subprocess
import sys

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

executor, source, output = sys.argv[1:4]
subprocess.run([executor, '--no-show', '--output', output, source], check=True)
script = pathlib.Path(output).read_text(encoding='utf-8')
assert 'target.errorbar(_mid, y, xerr=_xerr' in script
assert 'target.errorbar(x, _mid, yerr=_yerr' in script
assert 'plt.show()' not in script

namespace = {'__name__': '__main__'}
exec(compile(script, output, 'exec'), namespace)
fig = namespace['fig']
assert len(fig.axes) == 4
fig.canvas.draw()


def bounds(ax, axis):
    """Read error-bar endpoint coordinates from a Matplotlib LineCollection."""
    assert len(ax.containers) == 1
    container = ax.containers[0]
    assert container.has_xerr == (axis == 'x')
    assert container.has_yerr == (axis == 'y')
    segments = container.lines[2][0].get_segments()
    i = 0 if axis == 'x' else 1
    return [(round(float(seg[0][i]), 12), round(float(seg[1][i]), 12))
            for seg in segments]


# The actual coefficient estimate must remain at its own x coordinate.
first = fig.axes[0]
assert bounds(first, 'x') == [(0.3, 0.7), (0.75, 1.45), (1.9, 2.1)]
assert [t.get_text() for t in first.get_yticklabels()] == [
    'Baseline', 'With controls', 'Robust']
assert [round(float(v), 12) for v in first.collections[-1].get_offsets()[:, 0]] == [
    0.5, 1.1, 2.0]
assert tuple(first.containers[0].lines[2][0].get_colors()[0])[:3] == (0., 0., 0.)

# The interval midpoint differs from the point estimate in this panel.
second = fig.axes[1]
assert bounds(second, 'x') == [(0.4, 1.1), (1.5, 2.3)]
assert [round(float(v), 12) for v in second.collections[-1].get_offsets()[:, 0]] == [2.0, 3.0]

assert bounds(fig.axes[2], 'x') == [(0.8, 1.2), (1.4, 1.6)]
assert bounds(fig.axes[3], 'y') == [(0.9, 1.9), (1.2, 1.6)]
plt.close(fig)
