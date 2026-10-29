"""Regression: real libgretl panplot --stack produces six vertically stacked axes."""
import os
import pathlib
import subprocess
import sys

exe, source, output = sys.argv[1:4]
env = dict(os.environ, MPLBACKEND='Agg')
subprocess.run([exe, '--emit-python', '--output', output, source], check=True, env=env)
script = pathlib.Path(output)
code = script.read_text()
assert 'plt.subplots(6, 1' in code, 'stack layout must be 6 by 1'
assert code.count('MultipleLocator(') >= 6, 'six panel-specific y tick intervals expected'
exec(compile(code, str(script), 'exec'), {'__name__': '__main__'})
