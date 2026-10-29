#!/usr/bin/env python3
"""Regression test for bare gnuplot text commands that clear labels."""
import subprocess
import sys
import tempfile
from pathlib import Path

executor = sys.argv[1]
with tempfile.TemporaryDirectory() as temp:
    source = Path(temp) / "empty-text.gp"
    source.write_text(
        "set title 'Old title'\n"
        "set xlabel 'Old X'\n"
        "set ylabel 'Old Y'\n"
        "set y2label 'Old Y2'\n"
        "set title\n"
        "set xlabel\n"
        "set ylabel\n"
        "set y2label\n"
        "plot '-' using 1:2 with lines\n"
        "1 2\n2 3\ne\n"
    )
    result = subprocess.run([executor, "--emit-python", str(source)], capture_output=True, text=True)
    assert result.returncode == 0, result.stderr
    output = source.with_suffix(".py").read_text()
    for label in ("Old title", "Old X", "Old Y", "Old Y2"):
        assert label not in output, label
print("empty text commands: OK")
