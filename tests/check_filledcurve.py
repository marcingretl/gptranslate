#!/usr/bin/env python3
"""Verify gretl confidence-band expressions and their Matplotlib translation."""
import ast
import pathlib
import subprocess
import sys

executor, source, output = sys.argv[1:]
subprocess.run([executor, '--emit-python', source], check=True)
script = pathlib.Path(source).with_suffix('.py')
text = script.read_text()
# Filled bands must be rendered behind points and lines.
# Verify the configured fill opacity and absence of a border.
assert 'target.fill_between(x, y, upper, alpha=' in text
assert 'linewidth=0, zorder=0, **kw)' in text
assert "kw['color'] = '#DDDDDD'" in text
assert 'target.scatter(x, y' in text
# Verify that the first x coordinate is a data column, not literal 1.
first_x = next(line for line in text.splitlines() if line.startswith('x = '))
assert ast.literal_eval(first_x[4:])[0] == 1974
first_y = next(line for line in text.splitlines() if line.startswith('y = '))
assert abs(ast.literal_eval(first_y[4:])[0] - (11.63255023 - 0.1)) < 1e-9
upper = next(line for line in text.splitlines() if line.startswith('upper = '))
assert abs(ast.literal_eval(upper[8:])[0] - (11.63255023 + 0.1)) < 1e-9
pathlib.Path(output).write_text(text)
