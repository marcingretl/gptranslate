"""Check numeric tic sequences in the generated Matplotlib script."""
import ast
import pathlib
import subprocess
import sys

executor, source, output = sys.argv[1:]
result = subprocess.run([executor, f"--output={output}", source], capture_output=True, text=True)
assert result.returncode == 0, result.stderr
code = pathlib.Path(output).read_text(encoding="utf-8")
ast.parse(code)
assert "MultipleLocator(5)" in code
assert "FixedLocator([1 + i * 22" in code
