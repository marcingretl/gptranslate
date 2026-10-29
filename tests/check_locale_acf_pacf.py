#!/usr/bin/env python3
"""Check decimal-comma ACF/PACF panels and ordinary comma-separated plots."""
import pathlib
import re
import subprocess
import sys
import tempfile

exe, gp = sys.argv[1:]
with tempfile.TemporaryDirectory() as tmp:
    out = pathlib.Path(tmp) / 'acf.py'
    subprocess.run([exe, '--output', str(out), gp], check=True)
    script = out.read_text()
    compile(script, str(out), 'exec')
    assert script.count('target.vlines(x, 0, y, **kw)') == 2
    assert 'x = [1,2,3,4]' in script
    assert '0.918362' in script
    assert '-0.246155' in script
    assert script.count('target.axhline(0.264286') == 2
    assert script.count('target.axhline(-0.264286') == 2
    assert script.count('ax.set_ylim(bottom=-1)') == 2
    assert script.count('ax.set_ylim(top=1)') == 2
    assert "'+- 1,96/T^0,5'" in script

    # A dot-decimal plot still treats a comma as a series separator.
    standard = pathlib.Path(tmp) / 'standard.gp'
    standard.write_text('plot 0.25 title "upper", -0.25 title "lower"\n')
    other = pathlib.Path(tmp) / 'standard.py'
    subprocess.run([exe, '--output', str(other), str(standard)], check=True)
    normal = other.read_text()
    assert 'target.axhline(0.25' in normal
    assert 'target.axhline(-0.25' in normal
