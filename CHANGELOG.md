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
- Fixed parsing of gnuplot tic modifiers with a spaced font specification
  such as `font ", 8"` (found in `PanelTools`). A headless regression
  checks the resulting tick-label font size.
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
- On 2026-11-17, the `priority8` custom audit completed **8/8** cases for
  **5** additional packages (`VARrec`, `johansenrec`, `BEKK`, `MSVAR`,
  `PanelTools`) and generated **11** Matplotlib figures. Combined
  `priority5`–`priority8` integration results: **40/40 PASS**, **45**
  figures across **30** packages. `MSVAR` needed a suite-specific dataset
  preload before package inclusion; `PanelTools` needed the tic-font
  parsing correction above. Plot creation does not establish pixel-identical
  output against gnuplot.
- As of 2026-11-18, the `priority9` custom audit had completed its entire
  **8/8**-case suite with no errors or skips: four graphical paths in `qcorr`
  and four in `lagreg`, generating **8** Matplotlib figures. The maintained
  `priority5`–`priority9` suites now total **48/48 PASS**, **53** figures
  across **32** distinct contributed packages. No parser changes were needed
  for `priority9`. These figures have not been established as pixel-identical
  to gnuplot output.

- On 2026-11-18, the `priority10` custom audit completed **8/8** cases
  for two previously untested packages: `DynMultCalc` (dynamic/cumulative
  multiplier plots at several confidence levels) and `ghosts` (spectrum,
  coherence, phase and combined-panel plots), producing **8** Matplotlib
  figures with zero failures or skips. Across maintained `priority5`–
  `priority10` cases, the confirmed total is **56/56 PASS**, **61** figures
  from **34** distinct contributed packages. `priority10` did not require
  a new parser change. This does not establish pixel-identical rendering.


- On 2026-11-18, the `priority11` custom audit completed **8/8** cases
  for two additional packages, `FEP` and `PTconf`, producing **10**
  Matplotlib figures: four asymmetric loss-function plots in `FEP`, and
  six GG/SWP transitory-component plots in `PTconf`. The full run recorded
  zero failures or skips. Across `priority5`–`priority11`, maintained
  integration coverage is now **64/64 PASS**, **71** Matplotlib figures
  for **36** distinct packages. No `gptranslate` parser changes were
  required for `priority11`. Successful figure creation does not prove
  pixel-identical results relative to gnuplot.


- On 2026-11-18, the full `priority12` custom audit completed **8/8**
  cases for two additional packages, `PairPlot` and `ketvals`, generating
  **8** Matplotlib figures without failures or skips. The four `PairPlot`
  fixtures exercise triangular, matrix, row and column layouts, grouping
  and centroid labelling; the four `ketvals` fixtures cover time-varying
  OLS/IV coefficients with 90% and 95% confidence levels. Across
  `priority5`–`priority12` the maintained total is **72/72 PASS** and
  **79** figures from **38** distinct packages. No new `gptranslate`
  parser changes were necessary for `priority12`. Plot creation alone
  does not establish pixel identity with gnuplot.


- On 2026-11-18, the complete `priority13` custom audit passed **8/8**
  cases for the previously untested `QMLSV` and `TVC` packages, yielding
  **13** Matplotlib figures (five from QMLSV volatility/density and eight
  from TVC time-varying coefficient paths) with no failures or skips.
  The maintained `priority5`–`priority13` suites now total **80/80 PASS**
  and **92** figures from **40** distinct packages. No parser changes
  were required for `priority13`. Plot execution and figure generation
  do not establish visual identity with gnuplot.


- On 2026-11-18, the complete `priority14` contributed-package
  integration suite passed **8/8** tests with **8** Matplotlib figures:
  four `kmeans` cluster/scree/silhouette plots and four `GHegy` seasonal
  unit-root-test plots. The previous missing `GHegy` coefficient data
  and invalid assignment of the void-returning `kmeans_screeplot()` were
  resolved at installation/fixture level, without modifying the
  `gptranslate` parser or upstream package sources. The maintained
  `priority5`–`priority14` total is now **88/88 PASS**, **100** figures
  from **42** distinct packages. Plot creation does not demonstrate
  pixel-identical output relative to gnuplot.


- On 2026-11-18, the complete `priority15` contributed-package
  integration suite passed **8/8** tests with **15** Matplotlib figures:
  **11** from `BayTool` posterior density/MCMC diagnostics and **4** from
  `ParMA` marginal posterior and sampling diagnostics. Initial BayTool
  failures were caused by the test fixture passing a scalar `string` to
  `BT_paramplot` instead of a `strings` array. The corrected fixtures use
  `defarray(...)` and pass without modifying `gptranslate`, `gretl4py`
  or libgretl. Across `priority5`–`priority15`, maintained coverage is
  **96/96 PASS**, **115** figures and **44** distinct packages.
  Visual fidelity relative to gnuplot was not evaluated.


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
