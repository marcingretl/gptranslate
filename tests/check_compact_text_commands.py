#!/usr/bin/env python3
"""Regression test for gnuplot text commands without whitespace."""
import os
import subprocess
import tempfile
from pathlib import Path

executor = os.environ.get("GP_EXECUTOR", "gp-executor")
commands = (
    "set title' Posterior density estimate, Gaussian kernel, scale = 1'",
    'set xlabel"Horizontal axis"',
    "set ylabel'Vertical axis'",
    'set y2label"Secondary axis"',
)
with tempfile.TemporaryDirectory() as temp:
    source = Path(temp) / "compact.gp"
    source.write_text("\n".join(commands) + "\nplot '-' using 1:2 with lines\n1 2\n2 3\ne\n")
    result = subprocess.run([executor, "--emit-python", str(source)], capture_output=True, text=True)
    assert result.returncode == 0, result.stderr
    output = source.with_suffix(".py").read_text()
    for label in ("Posterior density estimate, Gaussian kernel, scale = 1", "Horizontal axis", "Vertical axis"):
        assert label in output, label
print("compact text commands: OK")
