"""Check SVAR IRF inline data and the abbreviated `with lines` style."""
import pathlib
import subprocess
import sys
import tempfile

exe, fixture = sys.argv[1:3]
with tempfile.TemporaryDirectory() as directory:
    script = pathlib.Path(directory) / "irf.py"
    subprocess.run([exe, "--output=" + str(script), fixture], check=True)
    generated = script.read_text()
    assert "target.plot(" in generated, "expected a line plot"
    assert "0.238243" in generated and "-0.0604878" in generated
    assert "linewidth" in generated and "2" in generated
    subprocess.run([sys.executable, str(script)], check=True,
                   env={**__import__("os").environ, "MPLBACKEND": "Agg"})
