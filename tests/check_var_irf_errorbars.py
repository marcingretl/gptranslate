#!/usr/bin/env python3
"""Check gretl VAR y-errorbars, asymmetric quantiles and direct Matplotlib execution."""
import pathlib
import subprocess
import sys

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

executor, source, output = sys.argv[1:4]
subprocess.run([executor, '--no-show', '--output', output, source], check=True)
script = pathlib.Path(output).read_text(encoding='utf-8')
assert "target.errorbar(x, _mid, yerr=_yerr" in script
assert "target.scatter(x, y, s=12" in script
assert 'plt.show()' not in script

namespace = {'__name__': '__main__'}
exec(compile(script, output, 'exec'), namespace)
fig = namespace['fig']
assert len(fig.axes) == 4
fig.canvas.draw()


def errorbar_bounds(ax):
    """Read the exact lower and upper whisker endpoints from Matplotlib."""
    assert len(ax.containers) == 1
    container = ax.containers[0]
    assert container.has_yerr and not container.has_xerr
    segments = container.lines[2][0].get_segments()
    return [(round(float(seg[0][1]), 12), round(float(seg[1][1]), 12))
            for seg in segments]


# The first point estimate lies ABOVE its confidence interval. Matplotlib's
# yerr relative to the estimate would be negative and therefore invalid.
first = fig.axes[0]
assert errorbar_bounds(first) == [(0.9, 1.9), (1.2, 1.6), (0.1, 0.6)]
assert list(first.lines[0].get_ydata()) == [2.0, 1.5, 0.5]
assert len(first.collections) >= 2  # Whiskers and actual point markers.
# A combined plot should not assign a contrasting hue to its whiskers.
line_color = matplotlib.colors.to_rgba(first.lines[0].get_color())
whisker_color = tuple(first.containers[0].lines[2][0].get_colors()[0])
assert line_color == whisker_color

# Symmetric y-error column: x, y, dy.
assert errorbar_bounds(fig.axes[1]) == [(0.8, 1.2), (1.4, 1.6), (0.9, 1.5)]
# Implicit 4-column input: x, y, lower, upper.
assert errorbar_bounds(fig.axes[2]) == [(-1.5, -0.8), (-1.4, -0.7), (-0.9, -0.6)]
# Arithmetic `using` expressions should give the same symmetric bounds.
assert errorbar_bounds(fig.axes[3]) == [(0.8, 1.2), (1.4, 1.6), (0.9, 1.5)]
plt.close(fig)
