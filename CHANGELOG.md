# Changelog

## Unreleased

- Added native Matplotlib hover tooltips for point and linespoint markers without third-party packages. Tooltips show Cartesian coordinates (including polar inverse roots), ignore bare lines, work with multiple axes and figures, and remain active when `--no-show` delegates display to the embedding application. Added headless motion-event regression tests.
- Added general support for `unset border`, `set yzeroaxis`, `unset ytics`/`set noytics`, `set size square` and `set polar` in the plot IR and Matplotlib backend. Polar inline data (theta in radians, radius) render in Cartesian coordinates, while a constant polar function draws a circle; polar analytic functions remain unsupported.
- Added headless VAR inverse-roots regression coverage for geometry, axes and tic resetting; the panplot-stack runtime test now uses `--no-show`.
- Replaced the experimental Qt-only option with `--backend=NAME`, which emits `matplotlib.use(NAME)` before importing `pyplot`; the old switch is no longer accepted. An empty backend preserves Matplotlib's default. `--backend=QtAgg` continues to preload PySide6. This can be combined with `--no-show` and does not change default output or plot-queue behavior. A working GUI event loop is still required for non-blocking display.
- Added `--no-show` to omit the final `plt.show()` from generated Matplotlib scripts. By default, generated scripts still include `plt.show()`.
- This option lets an embedding application such as gretl4py control figure display itself. Display behavior in non-interactive scripts and on Windows/TkAgg is still under investigation; no change to plot-queue handling is claimed.

## [0.12.0] - 2026-10-29

First public, experimental release.

- C++17 gnuplot parser, plot IR and Matplotlib Python-source backend.
- Coverage of the principal gretl plotting commands, with full `gridplot` compatibility remaining out of scope.
- Translation of a broad set of libgretl and gretl function-package plots.
- Named data blocks, matrix/image plots, palettes, fitted functions, filled curves, histograms, boxplots, multiple axes and positioned panels.
- gretl4py plot queue and libgretl compatibility/version-probe handling.
- Regression fixtures and tests for BACE, ParMA, SVAR, BVAR and core plotting constructs.
- Automatic contrasting colors for multi-series plots, with opt-out.

Earlier development milestones (0.4–0.11.11) were internal iterations before this public release. See Git history for details when available.
