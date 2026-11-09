#!/usr/bin/env python3
"""Check default display-neutral scripts, opt-in plt.show(), and queue behavior."""
import os
import pathlib
import subprocess
import sys
import tempfile

executor, source = sys.argv[1:]

with tempfile.TemporaryDirectory() as tmp:
    root = pathlib.Path(tmp)
    output = root / "plot.py"

    def generate(*flags):
        # The translator only writes Python; no GUI or Matplotlib is needed.
        result = subprocess.run([executor, *flags, "--output", str(output), source],
                                capture_output=True, text=True, check=True)
        assert not result.stdout, result.stdout
        code = output.read_text(encoding="utf-8")
        compile(code, str(output), "exec")
        return code

    default = generate()
    assert "fig, " in default or "fig = " in default
    assert "plt.show()" not in default
    assert generate("--add-show") == default + "plt.show()\n"
    assert generate("--no-show") == default
    # If both options are specified, the last option wins.
    assert generate("--add-show", "--no-show") == default
    assert generate("--no-show", "--add-show") == default + "plt.show()\n"

    help_result = subprocess.run([executor, "--help"], check=True,
                                 capture_output=True, text=True)
    assert "--add-show" in help_result.stdout

    queue = root / "queue"
    queue.mkdir()
    env = dict(os.environ, GRETL4PY_PLOT_QUEUE=str(queue))
    for _ in range(2):
        # Two independent -persist calls must publish executable, display-neutral files.
        subprocess.run([executor, "-persist", source], env=env,
                       check=True, capture_output=True)
    entries = sorted(queue.glob("*.py"))
    assert len(entries) == 2
    assert not list(queue.glob("*.tmp"))
    for entry in entries:
        code = entry.read_text(encoding="utf-8")
        compile(code, str(entry), "exec")
        assert "plt.show()" not in code

    # Explicit opt-in remains available even when queue output is requested.
    subprocess.run([executor, "-persist", "--add-show", source], env=env,
                   check=True, capture_output=True)
    queued_with_show = sorted(queue.glob("*.py"))
    assert len(queued_with_show) == 3
    last = (set(queued_with_show) - set(entries)).pop()
    assert last.read_text(encoding="utf-8").endswith("plt.show()\n")
