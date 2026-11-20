"""Verify libgretl geomap's ARG0-based external data-file reference."""
import ast
import pathlib
import subprocess
import sys
import tempfile

exe, fixture = sys.argv[1:3]
fixture = pathlib.Path(fixture).resolve()
with tempfile.TemporaryDirectory() as directory:
    script = pathlib.Path(directory) / "geomap.py"
    # Resolve the companion .dat file regardless of the working directory.
    result = subprocess.run([exe, "--output=" + str(script), str(fixture)],
                            capture_output=True, text=True, cwd=directory)
    assert result.returncode == 0, result.stderr
    generated = script.read_text(encoding="utf-8")
    ast.parse(generated)
    assert "target.plot(" in generated, "geomap coordinates not plotted"
    # An ARG0-based file cannot be resolved without a named source script.
    result = subprocess.run([exe, "--output=" + str(script)], input=fixture.read_text(),
                            capture_output=True, text=True, cwd=directory)
    assert result.returncode != 0 and "requires an input filename" in result.stderr
