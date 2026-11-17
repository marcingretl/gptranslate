# Changelog

## Unreleased

### Contributed-package plotting compatibility

- Added palette-based color mapping for `color_xy`, including `set palette negative`,
  mapped 2D `splot` data and `linespoints` colored via `lt palette`.
- Added gnuplot pie-chart constructs, including scalar arithmetic assignments,
  semicolon-separated statements, circle sectors (`set object ... circle arc`)
  and associated labels.
- Added candlestick/OHLC rendering, optionally with volume in a multiplot layout.
  Extended axis formatting for `set format x ("%.0f")` and gnuplot SI formats
  such as `\%.0s \%c`, including multiple escaped percent signs.
- Added Sankey-style graphical objects, including translucent rectangles,
  filled curves and polynomial/function segments.
- Added horizontal `xerrorbars`, using both symmetric offsets and asymmetric
  limits. Kept the estimate position independent of its error interval, and
  preserved the existing `errorbars`/`yerrorbars` behavior. Added support for
  the named color syntax `lc black` used by `coefcomp`.
- Extended explicit tic-list parsing to handle lists after modifiers such as
  `nomirror`, preserving quoted labels containing spaces, commas or parentheses
  (used by `coefcomp`).
- Extended label parsing and Matplotlib rendering to recognize `left`, `center`
  and `right` alignment and `front`, including mixed `graph` coordinates without
  repeating the `graph` keyword for the second coordinate (used by `roc`).
- Added bounded `set for [i=...:...] linetype i ps ...` statements, explicit
  `set linetype N ps ...`, and point-size overrides attached to plotted series
  (used by `season_plot`).
- Restored acceptance of decimal-comma scalar literals through `number()`;
  this fixes a regression in libgretl-generated boxplots.
- Extended contributed-package compatibility for `kddensity` gridded inline
  XYZ contour and `pm3d` map output, including `unset ztics`, bare
  `set contour`, `set dgrid3d X,Y` and `set view map scale S`.
- Preserved positive `set size ratio R` as a Matplotlib axes-box ratio for
  correspondence-analysis biplots, independently of `set size ratio -1`.
- Extended named `set style line N` parsing for the `naiveFC` package:
  `lc rgb "..." lt N` definitions no longer require an explicit `lw`.
  The Matplotlib backend retains the default line width when none is supplied.
- Added support for `with boxxy`/`boxxyerror` rectangles used in
  `random_forest`, including center/half-size and explicit-bounds forms,
  per-row dimensions, and category labels from `ytic(N)`. Constant arithmetic
  with previously assigned gnuplot scalars is now accepted in `set offsets`
  and relevant `using` expressions. The backend applies offsets to automatic
  axis limits even when the opposite limit is fixed explicitly.
- Extended named line styles to accept `ps` / `pointsize`, including
  point-size-only definitions such as `set style line 1 ps 0.02` used by
  `fsboost`. Specified point sizes are propagated to Matplotlib markers
  without overwriting unspecified line colors, types, or widths.

### Other changes since 0.12.0

- Fixed correspondence-analysis label parsing for `font",10"center`
  and `center tc rgb` modifiers, including labels without whitespace
  between font and alignment. Matplotlib now preserves per-label font size,
  centering and RGB text color. Added a headless label-geometry regression.

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

### Regression and integration verification

- Added headless translation/rendering regressions for the new constructs,
  including named axis tics, horizontal error bars, label placement, inline
  XYZ contours and heatmaps, and linetype point-size loops.
- On 2026-11-15, the full Meson test suite completed with **83 passed,
  0 failed**. The `priority5` audit completed **17/17** reviewed custom cases
  across **15** contributed packages, producing **19** Matplotlib figures.
- Added headless regression tests `naivefc-style-line`,
  `random-forest-boxxy`, `boxxy-bounds`, and `fsboost-style-pointsize`,
  covering the additional named-line styles, BoxXY geometry, category labels,
  and expression-based offsets.
- On 2026-11-17, the external `gretl4py` custom integration audits completed
  `priority5` **17/17**, `priority6` **7/7**, and `priority7` **8/8**:
  **32/32** reviewed cases across **25** packages, generating **34**
  Matplotlib figures with no failures. The latest full candidate-package
  audit separately reported **56 PASS, 0 FAIL** and **177** figures, with
  skipped and no-figure cases reported separately. The complete Meson test
  suite also passed after the `priority7` changes; its updated test count
  was not recorded here. Successful execution does not guarantee
  pixel-identical rendering against gnuplot.


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
