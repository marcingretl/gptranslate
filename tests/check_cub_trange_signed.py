"""Check signed symbolic pi bounds without evaluating arbitrary expressions."""
from pathlib import Path
import runpy
import subprocess
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

executor, source, output = sys.argv[1:]
result = subprocess.run([executor, "--no-show", f"--output={output}", source],
                        capture_output=True, text=True)
assert result.returncode == 0, result.stderr
plt.close("all")
runpy.run_path(output, run_name="__main__")
ax = plt.gcf().axes[0]
assert len(ax.lines) == 1
x = ax.lines[0].get_xdata()
y = ax.lines[0].get_ydata()
# At t=-pi and t=pi the unit circle is closed at (-1, 0).
assert len(x) == len(y) == 401
assert abs(x[0] + 1.0) < 1e-10
assert abs(y[0]) < 1e-10
assert abs(x[200] - 1.0) < 1e-10
assert abs(x[-1] + 1.0) < 1e-10
plt.close("all")

# Unknown bounds must remain translation errors, not be ignored.
invalid = Path(source).read_text(encoding="utf-8").replace("[-pi:pi]", "[-pi:unknown]")
invalid_source = Path(output).with_suffix(".invalid.gp")
invalid_source.write_text(invalid, encoding="utf-8")
try:
    result = subprocess.run([executor, "--no-show", f"--output={output}", str(invalid_source)],
                            capture_output=True, text=True)
    assert result.returncode != 0
    assert "invalid parametric trange" in result.stderr
finally:
    invalid_source.unlink(missing_ok=True)
