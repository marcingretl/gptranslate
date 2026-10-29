"""Regression test for SVAR FEVD stacked histograms and external data reuse."""
import pathlib
import subprocess
import sys
import tempfile

exe, fixture = sys.argv[1:3]
fixture = pathlib.Path(fixture).resolve()
with tempfile.TemporaryDirectory() as directory:
    script = pathlib.Path(directory) / 'fevd.py'
    # Run from the source tree so the relative external data path is resolvable.
    subprocess.run([exe, '--output=' + str(script), str(fixture)],
                   check=True, cwd=fixture.parent.parent)
    generated = script.read_text()
    assert generated.count('target.bar(') == 2, 'expected two FEVD histograms'
    assert generated.count('bottom=_hist_bottom') == 2, 'expected row stacking'
    assert 'alpha=0.25' in generated, 'fill density was lost'
    assert "unemp" in generated and "infl" in generated
    # The generated Python must also execute with a headless backend.
    subprocess.run([sys.executable, str(script)], check=True,
                   env={**__import__('os').environ, 'MPLBACKEND': 'Agg'})
