#!/usr/bin/env python3
"""Check isolated queue publication without importing matplotlib."""
import os
import pathlib
import subprocess
import sys
import tempfile

exe, first, second = sys.argv[1:]
with tempfile.TemporaryDirectory() as temp:
    queue = pathlib.Path(temp) / "queue"
    queue.mkdir()
    env = dict(os.environ, GRETL4PY_PLOT_QUEUE=str(queue))
    for script in (first, second):
        result = subprocess.run([exe, "-persist", script], env=env,
                                check=True, capture_output=True)
        assert not result.stdout, result.stdout
    entries = sorted(queue.glob("*.py"))
    assert len(entries) == 2, entries
    assert not list(queue.glob("*.tmp"))
    for entry in entries:
        source = entry.read_text()
        compile(source, str(entry), "exec")
    # Explicit output must not be redirected into the queue.
    explicit = pathlib.Path(temp) / "explicit.py"
    result = subprocess.run([exe, "-persist", "--output", str(explicit), first],
                            env=env, check=True, capture_output=True)
    assert not result.stdout, result.stdout
    assert explicit.is_file()
    assert len(list(queue.glob("*.py"))) == 2
