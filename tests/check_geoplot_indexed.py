"""Test libgretl geoplot's indexed palette polygons and ARG0 file variant."""
import ast
import pathlib
import subprocess
import sys
import tempfile

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

executor, named_fixture, external_fixture = sys.argv[1:4]

# The named block uses two blank rows; the external file uses libgretl's
# `# end of entity` marker plus one blank row. Both must preserve per-state z.
for filename in (named_fixture, external_fixture):
    with tempfile.TemporaryDirectory() as directory:
        script = pathlib.Path(directory) / "map.py"
        result = subprocess.run(
            [executor, "--output=" + str(script), filename],
            capture_output=True, text=True, cwd=directory,
        )
        assert result.returncode == 0, result.stderr
        generated = script.read_text(encoding="utf-8")
        ast.parse(generated)
        assert generated.count("ax.add_patch(_gp_Polygon") == 3
        # The line overlay must break at polygon and disconnected-ring boundaries.
        assert "float('nan')" in generated
        plt.close("all")
        exec(compile(generated, str(script), "exec"), {"__name__": "__main__"})
        fig = plt.gcf()
        assert len(fig.axes) == 2, "map and its colorbox expected"
        assert len(fig.axes[0].patches) == 3
        assert len(fig.axes[0].lines) == 1
        colors = [patch.get_facecolor() for patch in fig.axes[0].patches]
        assert colors[0] == colors[1] and colors[1] != colors[2]
        plt.close("all")

# A general loop must fail explicitly rather than being misread as a data source.
invalid = "set output 'test.png'\nplot for [i=1:10] '-' u 1:2 with lines\n"
with tempfile.TemporaryDirectory() as directory:
    result = subprocess.run(
        [executor, "--output=" + str(pathlib.Path(directory) / "invalid.py")],
        input=invalid, capture_output=True, text=True,
    )
    assert result.returncode != 0 and "unsupported plot iteration" in result.stderr
