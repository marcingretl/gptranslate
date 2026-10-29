"""Check that a translated gnuplot palette is accepted by Matplotlib."""
import os
from pathlib import Path
import subprocess
import sys

executor, source, generated = sys.argv[1:]
subprocess.run([executor, '--output=' + generated, source], check=True)
code = Path(generated).read_text(encoding='utf-8')
assert "'darkgreen'" in code, 'gnuplot dark-green was not normalized'
assert "'dark-green'" not in code, 'invalid Matplotlib color survived translation'
env = dict(os.environ, MPLBACKEND='Agg')
subprocess.run([sys.executable, generated], check=True, env=env)
