#!/usr/bin/env python3
"""Verify Threshold_Panel numeric dash loops and inherited line styling."""
from pathlib import Path
import subprocess
import sys
import tempfile
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.colors import to_hex

executor, source, output = sys.argv[1:4]

def translate(path, target):
    return subprocess.run([executor, '--no-show', '--output', str(target), str(path)],
                          capture_output=True, text=True)

result = translate(source, output)
assert result.returncode == 0, result.stderr
namespace = {'__name__': '__main__'}
exec(compile(Path(output).read_text(encoding='utf-8'), output, 'exec'), namespace)
fig = namespace['fig']
ax = fig.axes[0]
fig.canvas.draw()
assert len(ax.lines) == 5, len(ax.lines)
lines = ax.lines
for ln in lines:
    assert to_hex(ln.get_color()) == '#000000', ln.get_color()
for ln in lines[:4]:
    assert abs(ln.get_linewidth() - 2.0) < 1e-9, ln.get_linewidth()
assert lines[0].get_linestyle() == '-'
assert lines[1].get_linestyle() == '--'
assert lines[2].get_linestyle() == ':'
assert lines[3].get_linestyle() == '-.'
# Matplotlib normalizes custom dash sequences to '--' on readback;
# the generated tuple retains the six-element dash/dot pattern.
assert "kw['linestyle'] = (0, (6, 2, 1, 2, 1, 2))" in Path(output).read_text(encoding='utf-8')
assert lines[4].get_linestyle() == '--'
plt.close(fig)

# A later single-linetype declaration can override the loop's dash type.
# Malformed and unbounded directives must continue to be rejected.
with tempfile.TemporaryDirectory() as tmp:
    directory = Path(tmp)
    original = Path(source).read_text(encoding='utf-8')
    positive = directory / 'override.gp'
    positive.write_text(original.replace('set linetype 2 lc rgb "black" lw 2.0',
                                         'set linetype 2 lc rgb "red" lw 3 dt 1'))
    target = directory / 'override.py'
    result = translate(positive, target)
    assert result.returncode == 0, result.stderr
    namespace = {'__name__': '__main__'}
    exec(compile(target.read_text(encoding='utf-8'), str(target), 'exec'), namespace)
    line = namespace['fig'].axes[0].lines[1]
    assert line.get_linestyle() == '-'
    assert abs(line.get_linewidth() - 3.0) < 1e-9
    assert to_hex(line.get_color()) == '#ff0000'
    plt.close(namespace['fig'])
    # Plot-level dt/lw modifiers take precedence over the linetype defaults.
    plot_override = directory / 'plot_override.gp'
    plot_override.write_text(original.replace(
        "using 1:4 with lines title '95pct'",
        "using 1:4 with lines lt 4 dt 1 lw 2.5 title '95pct'"))
    result = translate(plot_override, directory / 'plot_override.py')
    assert result.returncode == 0, result.stderr
    namespace = {'__name__': '__main__'}
    src = directory / 'plot_override.py'
    exec(compile(src.read_text(encoding='utf-8'), str(src), 'exec'), namespace)
    line = namespace['fig'].axes[0].lines[2]
    assert line.get_linestyle() == '-'
    assert abs(line.get_linewidth() - 2.5) < 1e-9
    plt.close(namespace['fig'])
    for bad in ('set for [i=1:6] linetype i dashtype i',
                'set for [i=1:5] linetype i dashtype (i+1)'):
        broken = directory / 'broken.gp'
        broken.write_text(original.replace('set for [i=1:5] linetype i dashtype i', bad))
        result = translate(broken, directory / 'broken.py')
        assert result.returncode != 0, (bad, result.stdout, result.stderr)
print('Threshold_Panel dash-loop: OK')
