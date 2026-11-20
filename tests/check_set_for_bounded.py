#!/usr/bin/env python3
"""Check generic bounded set loops, ordered overrides and source diagnostics."""
from pathlib import Path
import subprocess
import sys
import tempfile

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.colors import to_hex

executor, fixture, target = sys.argv[1:4]


def translate(source, output):
    return subprocess.run([executor, "--no-show", "--output", str(output), str(source)],
                          capture_output=True, text=True)


result = translate(fixture, target)
assert result.returncode == 0, result.stderr
namespace = {"__name__": "__main__"}
exec(compile(Path(target).read_text(encoding="utf-8"), target, "exec"), namespace)
fig = namespace["fig"]
ax = fig.axes[0]
fig.canvas.draw()
assert len(ax.lines) == 6, len(ax.lines)
for line in ax.lines[:3]:
    assert to_hex(line.get_color()) == "#336699", line.get_color()
    assert abs(line.get_linewidth() - 2) < 1e-9, line.get_linewidth()
assert [line.get_linestyle() for line in ax.lines[3:]] == [":", "-.", "--"]
# The variable is substituted in coordinates but not inside quoted labels.
assert sorted((label.get_text(), label.get_position()) for label in ax.texts) == [
    ("q", (1, 1)), ("q", (2, 1))
]
plt.close(fig)

# Malformed loops, excessive ranges, and unsupported expanded directives
# must all produce errors referring to the original source line.
with tempfile.TemporaryDirectory() as tmp:
    temporary = Path(tmp)
    failures = (
        "set for [i=1:*] linetype i dt 1",
        "set for [i=1:5:0] linetype i dt 1",
        "set for [i=1:5:-1] linetype i dt 1",
        "set for [i=1:300] linetype i dt 1",
        "set for [i=1:5] xlabel 'i'",
        "set for [i=1:3] unknown_command i",
        "set for [i=1:3] for [j=1:3] linetype i dt j",
        "set for [i=1:5] linetype i dt (i+1)",
    )
    for statement in failures:
        source = temporary / "invalid.gp"
        source.write_text("set output 'invalid.png'\n# remark\n" + statement + "\n"
                          "plot '-' using 1:2 with lines\n0 0\n1 1\ne\n", encoding="utf-8")
        result = translate(source, temporary / "invalid.py")
        assert result.returncode != 0, (statement, result.stdout, result.stderr)
        assert "error:3:" in result.stderr, (statement, result.stderr)
print("bounded set-for expansion: OK")
