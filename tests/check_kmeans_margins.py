#!/usr/bin/env python3
"""Verify character-unit and screen-unit gnuplot margin geometry."""
from pathlib import Path
import ast
import subprocess
import sys

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

exe, cases_dir, out_dir = sys.argv[1:]


def run_case(name):
    output = Path(out_dir) / (name + '.py')
    source = Path(cases_dir) / (name + '.gp')
    run = subprocess.run([exe, '--no-show', '--output', str(output), str(source)],
                         capture_output=True, text=True)
    assert run.returncode == 0, run.stderr
    code = output.read_text(encoding='utf-8')
    ast.parse(code)
    namespace = {'__name__': '__main__'}
    exec(compile(code, str(output), 'exec'), namespace)
    fig = namespace['fig']
    fig.canvas.draw()
    return fig, namespace


fig, namespace = run_case('kmeans-margins')
assert len(fig.axes) == 1
assert len(namespace['_gp_manual_margins']) == 1
position = fig.axes[0].get_position()
character_h = plt.rcParams['font.size'] / (72 * fig.get_size_inches()[1])
assert abs(position.y1 - (1 - character_h)) < 1e-8, position
assert abs(position.y0 - 3 * character_h) < 1e-8, position
plt.close(fig)

fig, namespace = run_case('kmeans-margins-screen')
position = fig.axes[0].get_position()
assert all(abs(a-b) < 1e-8 for a,b in zip(position.bounds, (0.1, 0.2, 0.8, 0.65))), position
plt.close(fig)

fig, namespace = run_case('kmeans-margins-multiplot')
assert len(fig.axes) == 2
assert len(namespace['_gp_manual_margins']) == 2
upper, lower = (ax.get_position() for ax in fig.axes)
assert upper.y1 > lower.y1
assert abs(upper.y1 - (1 - character_h)) < 1e-8, upper
assert abs(lower.y1 - (0.5 - character_h)) < 1e-8, lower
plt.close(fig)

fig, namespace = run_case('kmeans-margins-reset')
assert '_gp_manual_margins' not in namespace
plt.close(fig)

bad = Path(out_dir) / 'kmeans-invalid-margin.gp'
bad.write_text("set tmargin banana\nplot '-' using 1:2 with lines notitle\n0 0\n1 1\ne\n")
result = subprocess.run([exe, '--no-show', '--output', str(Path(out_dir) / 'invalid.py'), str(bad)],
                        capture_output=True, text=True)
assert result.returncode != 0
assert 'invalid margin directive' in result.stderr, result.stderr
print('kmeans margins: OK')
