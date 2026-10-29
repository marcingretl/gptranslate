"""Check that analytic fit expressions survive translation and evaluation."""
import ast
import pathlib
import subprocess
import sys

executor, source, output = sys.argv[1:]
result = subprocess.run([executor, f"--output={output}", source], capture_output=True, text=True)
assert result.returncode == 0, result.stderr
code = pathlib.Path(output).read_text(encoding="utf-8")
ast.parse(code)
assert code.count("def _gp_fit(value):") == 6
for expression in ("2*value", "4*value**2", "4*value**3", "3/value", "math.exp(", "math.log("):
    assert expression in code, expression
assert code.count("target.plot(x, y, zorder=3, **kw)") == 6
