"""Regression tests for log bases and right-hand axis tics."""
import ast
import pathlib
import subprocess
import sys

executor, fixture_dir, output_dir = map(pathlib.Path, sys.argv[1:])
output_dir.mkdir(parents=True, exist_ok=True)
for name, expected in (
    ("y2axis", "right_ax = ax.secondary_yaxis('right')"),
    ("ylogscale_default", "ax.set_yscale('log', base=10)"),
    ("ylogscale_base2", "ax.set_yscale('log', base=2)"),
):
    source = fixture_dir / (name + ".gp")
    target = output_dir / (name + ".py")
    result = subprocess.run([str(executor), f"--output={target}", str(source)],
                            capture_output=True, text=True)
    assert result.returncode == 0, f"{name}: {result.stderr}"
    code = target.read_text(encoding="utf-8")
    ast.parse(code)
    assert expected in code, f"{name}: missing {expected}"
    if name == "y2axis":
        assert "ax.tick_params(top=False, right=False)" in code
    print(f"PASS {name}")
