"""Verify the emitted Python without importing or executing matplotlib."""
import ast
import pathlib
import subprocess
import sys

executor, source, output = sys.argv[1:]
path = pathlib.Path(output)
path.unlink(missing_ok=True)
result = subprocess.run([executor, f"--output={output}", source], capture_output=True, text=True)
assert result.returncode == 0, result.stderr
assert path.is_file(), "no Python file generated"
code = path.read_text(encoding="utf-8")
ast.parse(code)
assert "plt.show()" not in code, "unexpected interactive show in default output"
assert "savefig(" not in code, "unexpected file rendering"
assert "matplotlib.use('Agg')" not in code, "unexpected headless backend"
assert "import matplotlib.pyplot as plt" in code
print(path)
