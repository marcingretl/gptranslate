# Check affine OLS translation and the requested legend position.
import pathlib
import subprocess
import sys

exe, source, target = sys.argv[1:]
result = subprocess.run([exe, "--output", target, source], check=True,
                        capture_output=True, text=True)
assert not result.stdout, result.stdout
script = pathlib.Path(target).read_text()
assert "target.plot(x, y, zorder=3, **kw)" in script
assert "1.818661" in script
assert "ax.legend(handles, labels, loc='upper left')" in script
assert "target.axhline(0.929" not in script
compile(script, target, "exec")
