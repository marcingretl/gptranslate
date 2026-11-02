#!/usr/bin/env python3
"""Validate native Matplotlib hover for points, linespoints and polar roots."""
import pathlib
import subprocess
import sys

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.backend_bases import MouseEvent

executor, fixtures, output_dir = sys.argv[1:4]
fixtures = pathlib.Path(fixtures)
output_dir = pathlib.Path(output_dir)


def translated_figure(fixture):
    output = output_dir / (fixture.stem + '-hover.py')
    subprocess.run([executor, '--no-show', '--output', str(output), str(fixture)],
                   check=True)
    code = output.read_text(encoding='utf-8')
    assert 'plt.show()' not in code, 'regression tests must not open windows'
    namespace = {'__name__': '__main__'}
    exec(compile(code, str(output), 'exec'), namespace)
    fig = namespace['fig']
    fig.canvas.draw()
    return fig, namespace


def move_mouse(fig, ax=None, xy=None):
    """Dispatch a real Matplotlib motion event in display (pixel) coordinates."""
    if ax is None:
        px, py = 0, 0
    else:
        px, py = ax.transData.transform(xy)
    event = MouseEvent('motion_notify_event', fig.canvas, int(round(px)), int(round(py)))
    fig.canvas.callbacks.process('motion_notify_event', event)


def current_tooltips(ax):
    return [text for text in ax.texts if text.get_visible() and ',' in text.get_text()]


# A single figure-level callback supports both Line2D and PathCollection.
fig, ns = translated_figure(fixtures / 'hover-points.gp')
assert len(fig.axes) == 2
assert len(ns['_gp_hover_artists']) == 2
assert len(ns['_gp_hover_annotations']) == 0  # Created only on first hover.
first, second = fig.axes

move_mouse(fig, first, (1, 1))  # linespoints marker
assert len(current_tooltips(first)) == 1
assert current_tooltips(first)[0].get_text() == '1.00, 1.00'

# A line segment between markers is not a selectable marker.
move_mouse(fig, first, (0.5, 0.5))
assert not current_tooltips(first)

# A regular line has no hover behavior, even at its vertices.
move_mouse(fig, first, (1, 2))
assert not current_tooltips(first)

# Switching panels hides the first annotation and shows the second.
move_mouse(fig, second, (2, 4))
assert not current_tooltips(first)
assert current_tooltips(second)[0].get_text() == '2.00, 4.00'
move_mouse(fig)
assert not current_tooltips(second)
plt.close(fig)

# The inverse-roots data are polar in gnuplot but Cartesian in Matplotlib.
fig, ns = translated_figure(fixtures / 'inverse-roots.gp')
ax = fig.axes[0]
assert len(ns['_gp_hover_artists']) == 1  # Unit circle/zero axes excluded.
move_mouse(fig, ax, (0.5, 0.5))
assert current_tooltips(ax)[0].get_text() == '0.50, 0.50'
move_mouse(fig, ax, (0, -1))  # Unit circle without a marker.
assert not current_tooltips(ax)
plt.close(fig)

# Repeated scripts can share Python globals (as with py::eval_file in gretl4py).
# A callback on an older window must still use its own figure and artists.
shared = {'__name__': '__main__'}
figures = []
for filename in ('inverse-roots.gp', 'hover-points.gp'):
    source = fixtures / filename
    target = output_dir / (source.stem + '-shared.py')
    subprocess.run([executor, '--no-show', '--output', str(target), str(source)],
                   check=True)
    exec(compile(target.read_text(encoding='utf-8'), str(target), 'exec'), shared)
    figures.append(shared['fig'])
    figures[-1].canvas.draw()
older_fig, newer_fig = figures
move_mouse(older_fig, older_fig.axes[0], (0.5, 0.5))
assert current_tooltips(older_fig.axes[0])[0].get_text() == '0.50, 0.50'
assert not current_tooltips(newer_fig.axes[0])
move_mouse(newer_fig, newer_fig.axes[0], (1, 1))
assert current_tooltips(newer_fig.axes[0])[0].get_text() == '1.00, 1.00'
assert current_tooltips(older_fig.axes[0])[0].get_text() == '0.50, 0.50'
plt.close(older_fig)
plt.close(newer_fig)

# A lines-only plot should not acquire any hover callback or annotations.
fig, ns = translated_figure(fixtures / 'svar-irf.gp')
assert '_gp_hover_artists' not in ns
assert not any(text.get_text().count(', ') == 1 for ax in fig.axes for text in ax.texts)
plt.close(fig)
