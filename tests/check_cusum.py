"""Check that analytic CUSUM limits use the inline observations' x range."""
import ast
import runpy
import subprocess
import sys
from pathlib import Path

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

executor, source, output = sys.argv[1:4]
subprocess.run([executor, f'--output={output}', source], check=True)
code = Path(output).read_text(encoding='utf-8')
ast.parse(code)
assert 'plt.show()' not in code, 'default output must remain display-neutral'

# The confidence bounds precede the data in gnuplot; both need x=1975..1987.5.
namespace = runpy.run_path(output, run_name='__cusum_test__')
fig = namespace['fig']
ax = fig.axes[0]
lines = ax.get_lines()
assert len(lines) == 4, f'expected two bounds, CUSUM and zero axis: {len(lines)}'
upper, lower, cusum, zero = lines
assert len(upper.get_xdata()) == len(lower.get_xdata()) == 200
for boundary in (upper, lower):
    x = boundary.get_xdata()
    assert abs(float(x[0]) - 1975.0) < 1e-10, x[0]
    assert abs(float(x[-1]) - 1987.5) < 1e-10, x[-1]
assert abs(float(upper.get_ydata()[0]) - 6.77007) < 1e-8
assert abs(float(upper.get_ydata()[-1]) - 20.044695) < 1e-8
assert abs(float(lower.get_ydata()[0]) + 6.77007) < 1e-8
assert abs(float(lower.get_ydata()[-1]) + 20.044695) < 1e-8

# Keep the observed path and the distinct gnuplot line colors.
assert abs(float(cusum.get_xdata()[0]) - 1975) < 1e-10
assert abs(float(cusum.get_xdata()[-1]) - 1987.5) < 1e-10
assert abs(float(cusum.get_ydata()[0]) - 0.246397) < 1e-8
assert abs(float(cusum.get_ydata()[-1]) - 7.90492) < 1e-8
assert upper.get_color().lower() == lower.get_color().lower() == '#d95f02'
assert cusum.get_color().lower() == '#1b9e77'
assert all(abs(float(y)) < 1e-10 for y in zero.get_ydata())
assert ax.get_ylim()[0] < -20 < 0 < 20 < ax.get_ylim()[1]
assert ax.get_xlim()[0] < 1975 < 1987.5 < ax.get_xlim()[1]
plt.close('all')
