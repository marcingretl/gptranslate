# Changelog

## Unreleased

- Added support for the `color_xy` package's palette-colored `splot` with
  `set view map`: a third numeric column now colors XY lines and points,
  including `linespoints` and the corresponding colorbar. `set palette
  negative` reverses either a defined or the default Matplotlib palette;
  `set palette positive` and bare `set palette` restore forward mapping.
  Added a headless regression covering color direction, segment coordinates,
  marker values, normalization and colorbar presence. The built-in palette
  remains an approximation of gnuplot's default colors.

- Improved `gridplot` compatibility with `set multiplot next` for skipped cells and
  figure-level titles in `set multiplot layout ... title ...`. The parser preserves
  panel indices across `reset`, while the Matplotlib backend hides unused cells
  and reserves space for `fig.suptitle()`. Added a headless 2x3 regression test.

- Fixed libgretl `freq` overlays with theoretical normal and gamma densities.
  The analytic parser now resolves previously assigned scalar parameters,
  recognizes gnuplot `pi` and `lgamma`, and preserves scalar assignments
  across conditional selection. The FREQ gamma formula is evaluated in log
  space to prevent overflow for large shape parameters (e.g., near 5952).
  Added normal/gamma fixtures reconstructed from `graphing.c` and headless
  tests of histogram normalization, density values and figure generation.

- Fixed the x domain for analytic functions that appear before inline observations in the same plot command (libgretl CUSUM confidence bounds). When no explicit `set xrange` is supplied, the functions now use the full x range of the plot data rather than Matplotlib's default `(0, 1)` limits. Added a real CUSUM fixture and a headless regression covering both confidence bounds, observations, colors and the zero reference axis.

- Added libgretl contingency-table heatmap compatibility: `set size ratio -1` (equal x/y data-unit scaling), `set palette model RGB`, normalized numeric RGB triples in `set palette defined`, colorbar titles via `set cblabel`, `unset label`, and `set label "..." at x,y center front`. Semicolon-separated commands on one source line are now expanded outside named data blocks while preserving source line diagnostics. Added a real 3x3 confusion-matrix fixture and a headless Matplotlib runtime regression checking exact matrix values, color-scale range, color stops, colorbar title and all nine cell labels.

- **Changed default display policy:** generated Python scripts no longer append `plt.show()` unless explicitly requested with the new `--add-show` option. This also makes `-persist` plot-queue entries display-neutral by default, allowing gretl4py to evaluate the whole batch before invoking non-blocking display according to its `auto_show` policy. `--no-show` remains accepted as a backward-compatible no-op for the default. Updated CLI help, README and regression tests for direct translation, selected backends and queued plots. This change supersedes the earlier `--no-show` opt-out behavior described below.

- Added `with errorbars` / `with yerrorbars` translation for libgretl VAR impulse-response charts, including three-column symmetric and four-column asymmetric errors, exact bootstrap quantile bounds even when the estimate falls outside the interval, and matched line/whisker colors. Added headless regression tests.
- Added native Matplotlib hover tooltips for point and linespoint markers without third-party packages. Tooltips show Cartesian coordinates (including polar inverse roots), ignore bare lines, work with multiple axes and figures, and remain active when `--no-show` delegates display to the embedding application. Added headless motion-event regression tests.
- Added general support for `unset border`, `set yzeroaxis`, `unset ytics`/`set noytics`, `set size square` and `set polar` in the plot IR and Matplotlib backend. Polar inline data (theta in radians, radius) render in Cartesian coordinates, while a constant polar function draws a circle; polar analytic functions remain unsupported.
- Added headless VAR inverse-roots regression coverage for geometry, axes and tic resetting; the panplot-stack runtime test now uses `--no-show`.
- Replaced the experimental Qt-only option with `--backend=NAME`, which emits `matplotlib.use(NAME)` before importing `pyplot`; the old switch is no longer accepted. An empty backend preserves Matplotlib's default. `--backend=QtAgg` continues to preload PySide6. This can be combined with `--no-show` and does not change default output or plot-queue behavior. A working GUI event loop is still required for non-blocking display.
- Initially added `--no-show` to omit the final `plt.show()` from generated Matplotlib scripts; at that stage the default still included `plt.show()`. The new `--add-show` opt-in above supersedes that behavior.
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
