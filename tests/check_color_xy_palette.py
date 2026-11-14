#!/usr/bin/env python3
"""Regress the color_xy package's flat palette-colored XYZ plots."""
import ast
from pathlib import Path
import subprocess
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.collections import LineCollection, PathCollection
import numpy as np


def translate(executor, source, output):
    """Translate a gnuplot file and execute only its generated plotting code."""
    result = subprocess.run([executor, "--no-show", "--output", str(output), str(source)],
                            capture_output=True, text=True)
    assert result.returncode == 0, result.stderr
    code = output.read_text(encoding="utf-8")
    ast.parse(code)
    namespace = {"__name__": "__main__"}
    exec(compile(code, str(output), "exec"), namespace)
    namespace["fig"].canvas.draw()
    return namespace


def check_palette(executor, source, output, *, cmap_name, points=True):
    """Verify coordinates, palette values, normalisation and the colorbar."""
    ns = translate(executor, source, output)
    fig, ax = ns["fig"], ns["ax"]
    try:
        assert len(fig.axes) == 2, "missing time-scale colorbar"
        assert len(ax.collections) == (2 if points else 1)
        line = ax.collections[0]
        assert isinstance(line, LineCollection)
        assert len(line.get_segments()) == 4
        assert np.allclose(line.get_segments()[0], [[0, 1], [1, 1.5]])
        assert np.allclose(line.get_array(), [2022.5, 2023.5, 2024.5, 2025.5])
        assert line.get_cmap().name == cmap_name
        assert line.norm.vmin == 2022 and line.norm.vmax == 2026
        if points:
            scatter = ax.collections[1]
            assert isinstance(scatter, PathCollection)
            assert len(scatter.get_offsets()) == 5
            assert np.allclose(scatter.get_array(), [2022, 2023, 2024, 2025, 2026])
            assert scatter.get_cmap().name == cmap_name
        assert ax.get_xlim()[0] < 0 < ax.get_xlim()[1]
        assert ax.get_xlim()[1] > 4
    finally:
        plt.close(fig)


def main():
    executor, fixture, output = sys.argv[1:]
    fixture, output = Path(fixture), Path(output)
    source = fixture.read_text(encoding="utf-8")
    assert "set palette negative" in source

    check_palette(executor, fixture, output, cmap_name="viridis_r")

    # Both positive and bare `set palette` must undo a prior reversal.
    for directive in ("set palette positive", "set palette"):
        modified = output.with_name("_color_xy_positive.gp")
        modified.write_text(source.replace("set palette negative", directive), encoding="utf-8")
        try:
            check_palette(executor, modified, output, cmap_name="viridis")
        finally:
            modified.unlink(missing_ok=True)

    # Explicit palette stops are reversed without affecting the numeric cbrange.
    modified = output.with_name("_color_xy_defined.gp")
    modified.write_text(source.replace("set palette negative",
                                       "set palette defined (0 'blue', 1 'red')\n"
                                       "set palette negative"), encoding="utf-8")
    try:
        ns = translate(executor, modified, output)
        try:
            cmap = ns["ax"].collections[0].get_cmap()
            assert np.allclose(cmap(0)[:3], [1, 0, 0])
            assert np.allclose(cmap(1.0)[:3], [0, 0, 1])
        finally:
            plt.close(ns["fig"])
    finally:
        modified.unlink(missing_ok=True)

    # Bare lines must remain color-mapped without adding point markers.
    modified = output.with_name("_color_xy_lines.gp")
    modified.write_text(source.replace("with linespoints lw", "with lines lw"), encoding="utf-8")
    try:
        check_palette(executor, modified, output, cmap_name="viridis_r", points=False)
    finally:
        modified.unlink(missing_ok=True)

    # A palette splot is flat only when `set view map` is explicit.
    invalid = output.with_name("_color_xy_no_map.gp")
    invalid.write_text(source.replace("set view map\n", ""), encoding="utf-8")
    try:
        result = subprocess.run([executor, "--no-show", "--output", str(output),
                                 str(invalid)], capture_output=True, text=True)
        assert result.returncode != 0
        assert "unsupported map-view palette splot" in result.stderr
    finally:
        invalid.unlink(missing_ok=True)

    print("color_xy palette trajectories: OK")


if __name__ == "__main__":
    main()
