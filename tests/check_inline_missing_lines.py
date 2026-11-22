#!/usr/bin/env python3
"""Ensure missing 2D samples do not connect otherwise valid line segments."""
from pathlib import Path
import math
import subprocess
import sys

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

executor, source, output = sys.argv[1:4]
result = subprocess.run([executor, '--no-show', '--output', output, source],
                        capture_output=True, text=True)
assert result.returncode == 0, result.stderr
code = Path(output).read_text(encoding='utf-8')
assert "float('nan')" in code
namespace = {'__name__': '__main__'}
exec(compile(code, output, 'exec'), namespace)
fig, ax = namespace['fig'], namespace['ax']
fig.canvas.draw()
lines = {line.get_label(): line for line in ax.lines}
assert set(lines) == {'lines', 'using-expression', 'linespoints', 'steps', 'named-header'}, set(lines)
for label, line in lines.items():
    x = list(map(float, line.get_xdata()))
    y = list(map(float, line.get_ydata()))
    assert x == [0, 1, 2, 3, 4, 5], (label, x)
    assert y[:2] == [1, 2] and y[4:] == [5, 6], (label, y)
    assert math.isnan(y[2]) and math.isnan(y[3]), (label, y)
plt.close('all')
print('Missing observations preserve line gaps: OK')
