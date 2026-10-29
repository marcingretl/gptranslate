#!/usr/bin/env python3
"""Verify that the SVAR historical-decomposition script keeps stacked histograms."""
import pathlib
import subprocess
import sys

exe, source, output = sys.argv[1:]
subprocess.run([exe, "--emit-python", "--output", output, source], check=True)
# The translator writes next to the source when invoked with --emit-python.
code = pathlib.Path(output).read_text()
assert code.count("target.bar(x, y, bottom=_hist_bottom") == 2
assert "_hist_positive = {}" in code
assert "_hist_negative = {}" in code
assert "target.plot(x, y, zorder=3" in code
assert "alpha=0.5" in code
compile(code, output, "exec")
