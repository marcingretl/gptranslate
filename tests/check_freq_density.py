"""Regression tests for libgretl's normal and gamma frequency overlays."""
import ast
import math
import runpy
import subprocess
import sys
from pathlib import Path

import matplotlib

matplotlib.use('Agg')
import matplotlib.pyplot as plt


executor, source, output, distribution = sys.argv[1:5]
subprocess.run([executor, f'--output={output}', source], check=True)
code = Path(output).read_text(encoding='utf-8')
ast.parse(code)
assert 'plt.show()' not in code, 'generation must remain display-neutral'

namespace = runpy.run_path(output, run_name='__freq_density_test__')
ax = namespace['fig'].axes[0]
assert len(ax.patches) == 7, 'histogram must contain all seven bins'
assert len(ax.lines) == 1, 'expected exactly one theoretical density curve'
assert len(ax.get_legend().get_texts()) == 2, 'both histogram and density need legend entries'
assert ax.get_ylabel() == 'Density'
assert abs(ax.patches[0].get_height() - 12 / (55 * 0.082)) < 1e-10

line = ax.lines[0]
xx = list(map(float, line.get_xdata()))
yy = list(map(float, line.get_ydata()))
assert len(xx) == len(yy) == 200
assert all(math.isfinite(y) for y in yy), 'no NaN/overflow in theoretical density'
assert all(y >= 0 for y in yy)
mu = 11.7544
sd = 0.152357
if distribution == 'normal':
    assert abs(xx[0] - 11.2) < 1e-10
    assert abs(xx[-1] - 12.4) < 1e-10
    peak = 1.0 / (sd * math.sqrt(2 * math.pi))
    assert abs(max(yy) - peak) < 0.001
    assert 'N(' in line.get_label()
elif distribution == 'gamma':
    # A shape near 5952 makes the direct x**(alpha-1)/Gamma(alpha)
    # formula overflow even though the distribution's PDF is finite.
    beta = sd**2 / mu
    alpha = mu / beta
    assert alpha > 5000
    assert abs(xx[0]) < 1e-12
    assert abs(xx[-1] - 12.4) < 1e-10
    assert 2.5 < max(yy) < 2.7
    mode = (alpha - 1) * beta
    index = min(range(len(xx)), key=lambda i: abs(xx[i] - mode))
    expected = math.exp((alpha-1) * math.log(xx[index]) - xx[index]/beta
                        - math.lgamma(alpha) - alpha * math.log(beta))
    assert abs(yy[index] - expected) < 1e-5
else:
    raise AssertionError('unknown test distribution')

print(f'freq-{distribution}: {len(ax.patches)} bars, finite density, peak {max(yy):.6f}')
plt.close('all')
