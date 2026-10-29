#!/usr/bin/env python3
"""Check that box widths are emitted in data coordinates."""
import pathlib
import subprocess
import sys

exe, gp, output = sys.argv[1:]
# The executor writes to the explicit output path and keeps stdout empty.
result = subprocess.run([exe, '--emit-python', '--output', output, gp], check=True,
                        capture_output=True, text=True)
assert not result.stdout
generated = pathlib.Path(output).read_text()
assert '_box_spacing = min(_box_steps)' in generated
assert 'width=_box_width' in generated
compile(generated, output, 'exec')
