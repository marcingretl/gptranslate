"""Check that the zero reference line does not expand the plotted y range."""
import runpy
import subprocess
import sys
from pathlib import Path

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

executor, source, output = sys.argv[1:4]
subprocess.run([executor, '--output=' + output, source], check=True)
plt.show = lambda *args, **kwargs: None
runpy.run_path(output)
ax = plt.gca()
low, high = ax.get_ylim()
assert 10 < low < 12 < high < 13, (low, high)
# The out-of-range zero axis must not create an extra horizontal artist.
assert not any(line.get_ydata()[0] == 0 for line in ax.lines if len(line.get_ydata())), ax.lines
plt.close('all')
