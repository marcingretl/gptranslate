#!/usr/bin/env python3
"""Check that filled boxes stay behind overlaid density curves."""
import pathlib
import subprocess
import sys
import tempfile

exe, gp = sys.argv[1:]
# Use a temporary output file rather than reading a path from stdout.
with tempfile.TemporaryDirectory() as tmp:
    output = pathlib.Path(tmp) / 'zorder.py'
    result = subprocess.run([exe, '--emit-python', '--output', str(output), gp],
                            check=True, capture_output=True, text=True)
    assert not result.stdout
    script = output.read_text()
assert 'target.bar(_box_centers, _box_heights, width=_box_widths, alpha=1, zorder=1, **kw)' in script
assert 'target.plot(x, y, zorder=3, **kw)' in script
compile(script, gp, 'exec')
