project('gptranslate', 'cpp', version: '0.12.0', default_options: ['cpp_std=c++17', 'warning_level=2'])
# Older libstdc++ versions (notably GCC 8) require -lstdc++fs for C++17
# std::filesystem. Test the actual link rather than the compiler version,
# since Clang can also use an older libstdc++ implementation.
cpp = meson.get_compiler('cpp')
filesystem_test = '''
#include <filesystem>
int main() {
    auto path = std::filesystem::current_path();
    return std::filesystem::exists(path) ? 0 : 1;
}
'''
filesystem_link_args = []
if not cpp.links(filesystem_test, name: 'std::filesystem without extra libraries')
  if cpp.links(filesystem_test, args: ['-lstdc++fs'], name: 'std::filesystem with -lstdc++fs')
    message('Linking gp-executor with -lstdc++fs')
    filesystem_link_args = ['-lstdc++fs']
  else
    error('C++17 std::filesystem is unavailable (also tried -lstdc++fs)')
  endif
endif

inc = include_directories('include')
core = static_library('gptranslate', ['src/parser.cpp', 'src/matplotlib.cpp'], include_directories: inc, install: false)
exe = executable('gp-executor', 'src/main.cpp', link_with: core, include_directories: inc, link_args: filesystem_link_args, install: true)

# Translation tests must not execute matplotlib or create PNG files.
py = import('python').find_installation('python3')
check = meson.current_source_dir() / 'tests' / 'check_translation.py'
test('basic', py, args: [check, exe.full_path(), meson.current_source_dir() / 'tests' / 'basic.gp', meson.current_build_dir() / 'basic-test.py'])
test('passengers', py, args: [check, exe.full_path(), meson.current_source_dir() / 'tests' / 'passengers.gp', meson.current_build_dir() / 'passengers-test.py'])

# libgretl's version query must continue working without Python/matplotlib.
test('version-probe', exe, args: [meson.current_source_dir() / 'tests' / 'gpver_query'], workdir: meson.current_build_dir())

# Named blocks are reusable data sources and do not consume inline rows.
test('named-block', py, args: [check, exe.full_path(), meson.current_source_dir() / 'tests' / 'named-block.gp', meson.current_build_dir() / 'named-block-test.py'])
test('named-block-reuse', py, args: [check, exe.full_path(), meson.current_source_dir() / 'tests' / 'named-block-reuse.gp', meson.current_build_dir() / 'named-block-reuse-test.py'])

# Matrix images use the same named-data-block infrastructure as line plots.
test('matrix-image', py, args: [check, exe.full_path(), meson.current_source_dir() / 'tests' / 'matrix-image.gp', meson.current_build_dir() / 'matrix-image-test.py'])

# Color-axis limits and piecewise palettes apply to generic matrix images.
test('palette-image', py, args: [check, exe.full_path(), meson.current_source_dir() / 'tests' / 'palette-image.gp', meson.current_build_dir() / 'palette-image-test.py'])

# Axis tics, secondary axes, and axis-link settings from libgretl scripts.
test('axis-tics', py, args: [check, exe.full_path(), meson.current_source_dir() / 'tests' / 'axis-tics.gp', meson.current_build_dir() / 'axis-tics-test.py'])

# Scalar assignments and conditional branches select only the active plot.
test('conditional-true', py, args: [check, exe.full_path(), meson.current_source_dir() / 'tests' / 'conditional-true.gp', meson.current_build_dir() / 'conditional-true-test.py'])
test('conditional-false', py, args: [check, exe.full_path(), meson.current_source_dir() / 'tests' / 'conditional-false.gp', meson.current_build_dir() / 'conditional-false-test.py'])

# Matrix labels use numeric formatting and conditional per-cell RGB colors.
test('matrix-labels', py, args: [check, exe.full_path(), meson.current_source_dir() / 'tests' / 'matrix-labels.gp', meson.current_build_dir() / 'matrix-labels-test.py'])

# Explicit multiplot panels inherit axis settings but not previously plotted data.
test('multiplot-positioned', py, args: [check, exe.full_path(), meson.current_source_dir() / 'tests' / 'multiplot-positioned.gp', meson.current_build_dir() / 'multiplot-positioned-test.py'])

# Constant-function confidence bounds and impulse series in two panels.
test('acf-pacf', py, args: [check, exe.full_path(), meson.current_source_dir() / 'tests' / 'acf-pacf.gp', meson.current_build_dir() / 'acf-pacf-test.py'])

# Verify positioned panels leave sufficient room for axis annotations.
test('multiplot-geometry', py, args: [meson.current_source_dir() / 'tests' / 'check_geometry.py', exe.full_path(), meson.current_source_dir() / 'tests' / 'acf-pacf.gp', meson.current_build_dir() / 'geometry-test.py'])

# Real libgretl/parma plots: solid fill with and without a border.
test('parma-histogram', py, args: [check, exe.full_path(), meson.current_source_dir() / 'tests' / 'parma-histogram.gp', meson.current_build_dir() / 'parma-histogram-test.py'])
test('parma-kernel', py, args: [check, exe.full_path(), meson.current_source_dir() / 'tests' / 'parma-kernel.gp', meson.current_build_dir() / 'parma-kernel-test.py'])

# Queue mode must preserve every -persist invocation and publish complete scripts.
test('plot-queue', py, args: [meson.current_source_dir() / 'tests' / 'check_queue.py', exe.full_path(), meson.current_source_dir() / 'tests' / 'parma-histogram.gp', meson.current_source_dir() / 'tests' / 'parma-kernel.gp'])

# Histogram widths must use x-axis data units, not a fixed matplotlib width.
test('parma-boxwidth', py, args: [meson.current_source_dir() / 'tests' / 'check_boxwidth.py', exe.full_path(), meson.current_source_dir() / 'tests' / 'parma-histogram.gp', meson.current_build_dir() / 'parma-boxwidth.py'])

# Dense histogram plateaus must become one rectangle per occupied bin.
test('parma-plateau', py, args: [meson.current_source_dir() / 'tests' / 'check_plateau.py', exe.full_path(), meson.current_source_dir() / 'tests' / 'parma-kernel.gp', meson.current_build_dir() / 'parma-plateau.py'])

# Filled histograms stay below density curves regardless of plotting order.
test('parma-zorder', py, args: [meson.current_source_dir() / 'tests' / 'check_zorder.py', exe.full_path(), meson.current_source_dir() / 'tests' / 'parma-kernel.gp'])

# Backend selection is opt-in and must precede pyplot without requiring GUI toolkits for translation tests.
test('backend-header', py, args: [meson.current_source_dir() / 'tests' / 'check_backend.py', exe.full_path(), meson.current_source_dir() / 'tests' / 'basic.gp'])

# The QtAgg-specific regression uses the supported --backend flag, not legacy --qtagg.
test('qtagg-header', py, args: [meson.current_source_dir() / 'tests' / 'check_qtagg.py', exe.full_path(), meson.current_source_dir() / 'tests' / 'basic.gp'])

# Default output and queued package plots must omit blocking plt.show().
test('show-mode', py, args: [meson.current_source_dir() / 'tests' / 'check_show_mode.py', exe.full_path(), meson.current_source_dir() / 'tests' / 'basic.gp'])

# Auto-contrast defaults on for combined plots and supports explicit opt-out.
test('auto-contrast', py, args: [meson.current_source_dir() / 'tests' / 'check_auto_contrast.py', exe.full_path(), meson.current_source_dir() / 'tests' / 'parma-kernel.gp'])

# Real BACE model-averaging plots exercise palette visibility, identity
# linked axes and gnuplot numeric tick formats.
foreach name : ['pip', 'msd', 'mpd', 'cdf']
  test('bace-' + name, py, args: [check, exe.full_path(), meson.current_source_dir() / 'tests' / ('bace-' + name + '.gp'), meson.current_build_dir() / ('bace-' + name + '-test.py')])
endforeach

# Execute the BACE PIP script with a headless Matplotlib backend so that
# invalid palette color names cannot pass syntax-only translation tests.
test('bace-pip-runtime', py, args: [meson.current_source_dir() / 'tests' / 'check_palette_runtime.py', exe.full_path(), meson.current_source_dir() / 'tests' / 'bace-pip.gp', meson.current_build_dir() / 'bace-pip-runtime.py'])

# Regression lines must retain their slope and requested legend placement.
test('regression-ols', py, args: [meson.current_source_dir() / 'tests' / 'check_regression_ols.py', exe.full_path(), meson.current_source_dir() / 'tests' / 'regression-ols.gp', meson.current_build_dir() / 'regression-ols.py'])

# Analytic fits cover polynomial, inverse and logarithmic expressions.
test('analytic-fits', py, args: [meson.current_source_dir() / 'tests' / 'check_analytic_fits.py', exe.full_path(), meson.current_source_dir() / 'tests' / 'analytic-fits.gp', meson.current_build_dir() / 'analytic-fits.py'])

# A gnuplot zero reference axis must not force the y range to include zero.
test('zeroaxis-autoscale', py, args: [meson.current_source_dir() / 'tests' / 'check_zeroaxis_autoscale.py', exe.full_path(), meson.current_source_dir() / 'tests' / 'regression-ols.gp', meson.current_build_dir() / 'zeroaxis-autoscale.py'])

# Real libgretl plots: logarithmic bases and right-hand axis tics.
# Check generated Python without displaying an interactive window.
test('logscale-y2axis', py, args: [meson.current_source_dir() / 'tests' / 'check_logscale_y2axis.py', exe.full_path(), meson.current_source_dir() / 'tests', meson.current_build_dir()])

# A real libgretl band plot uses arithmetic column expressions and filledcurve.
test('filledcurve', py, args: [meson.current_source_dir() / 'tests' / 'check_filledcurve.py', exe.full_path(), meson.current_source_dir() / 'tests' / 'filledcurve.gp', meson.current_build_dir() / 'filledcurve-test.py'])

# BVAR gridplot scripts use compact axis ranges and reset between panels.
test('bvar-reset', py, args: [check, exe.full_path(), meson.current_source_dir() / 'tests' / 'bvar-reset.gp', meson.current_build_dir() / 'bvar-reset-test.py'])

# libgretl boxplot: decimal commas, candlesticks, labels and disabled x tics.
test('boxplot', py, args: [check, exe.full_path(), meson.current_source_dir() / 'tests' / 'boxplot.gp', meson.current_build_dir() / 'boxplot-test.py'])

# Panel boxplot with median and dotted notch bounds (real libgretl output).
test('boxplot-notches', py, args: [check, exe.full_path(), meson.current_source_dir() / 'tests' / 'boxplot-notches.gp', meson.current_build_dir() / 'boxplot-notches-test.py'])

# Real libgretl panplot --stack: bare noxlabel and six numeric y-tic intervals.
test('panplot-stack', py, args: [meson.current_source_dir() / 'tests' / 'check_panplot_stack.py', exe.full_path(), meson.current_source_dir() / 'tests' / 'panplot-stack.gp', meson.current_build_dir() / 'panplot-stack-test.py'])

# Numeric tic sequences from libgretl: incr, start,incr, and start,incr,end.
test('tics-sequence', py, args: [meson.current_source_dir() / 'tests' / 'check_tics_sequence.py', exe.full_path(), meson.current_source_dir() / 'tests' / 'tics-sequence.gp', meson.current_build_dir() / 'tics-sequence.py'])

# SVAR FEVD uses external numeric data, reused via an empty filename, and
# rowstacked histograms selected by `set style data histogram`.
test('svar-fevd', py, args: [meson.current_source_dir() / 'tests' / 'check_fevd.py', exe.full_path(), meson.current_source_dir() / 'tests' / 'fevd-debug.gp'])

# SVAR impulse response plots use abbreviated line style and inline data.
test('svar-irf', py, args: [meson.current_source_dir() / 'tests' / 'check_svar_irf.py', exe.full_path(), meson.current_source_dir() / 'tests' / 'svar-irf.gp'])

# VAR inverse roots: polar theta/r data, unit circle, axes, square aspect and tics.
# Run headlessly without emitting plt.show(), including the tic reset checks.
test('inverse-roots', py, args: [meson.current_source_dir() / 'tests' / 'check_inverse_roots.py', exe.full_path(), meson.current_source_dir() / 'tests' / 'inverse-roots.gp', meson.current_build_dir() / 'inverse-roots-test.py'])

# Native Matplotlib hover displays data coordinates only for point markers,
# including polar roots and markers in independent subplot panels.
test('point-hover', py, args: [meson.current_source_dir() / 'tests' / 'check_hover.py', exe.full_path(), meson.current_source_dir() / 'tests', meson.current_build_dir()])

# libgretl VAR bootstrap IRFs: 4-column quantile whiskers, symmetric errors,
# implicit columns and arithmetic `using`, rendered headlessly by Matplotlib.
test('var-irf-errorbars', py, args: [meson.current_source_dir() / 'tests' / 'check_var_irf_errorbars.py', exe.full_path(), meson.current_source_dir() / 'tests' / 'var-irf-errorbars.gp', meson.current_build_dir() / 'var-irf-errorbars.py'])

# Residual periodograms use an independently scaled top axis for periods;
# the radians variant also uses symbolic pi positions on the bottom axis.
foreach kind : ['scaled', 'radians']
  test('periodogram-' + kind, py, args: [meson.current_source_dir() / 'tests' / 'check_periodogram.py', exe.full_path(), meson.current_source_dir() / 'tests' / ('periodogram-' + kind + '.gp'), meson.current_build_dir() / ('periodogram-' + kind + '.py')])
endforeach

# libgretl's graph-relative frequency annotations may use the front layer.
test('graphing-label-front', py, args: [check, exe.full_path(), meson.current_source_dir() / 'tests' / 'graphing-label-front.gp', meson.current_build_dir() / 'graphing-label-front.py'])

# Quarter and month time-series plots specify primary x minor-tic counts.
test('primary-minortics', py, args: [check, exe.full_path(), meson.current_source_dir() / 'tests' / 'primary-minortics.gp', meson.current_build_dir() / 'primary-minortics.py'])

# libgretl confidence ellipses use affine parametric equations, marginal
# interval bounds and a point annotation at the fitted coefficient pair.
test('confidence-ellipse', py, args: [meson.current_source_dir() / 'tests' / 'check_confidence_ellipse.py', exe.full_path(), meson.current_source_dir() / 'tests' / 'confidence-ellipse.gp', meson.current_build_dir() / 'confidence-ellipse.py'])

# libgretl contingency-table heatmap: equal aspect, RGB color stops,
# annotated cells, colorbar title and semicolon-separated commands.
test('confusion-matrix-heatmap', py, args: [meson.current_source_dir() / 'tests' / 'check_confusion_matrix.py', exe.full_path(), meson.current_source_dir() / 'tests' / 'confusion-matrix.gp', meson.current_build_dir() / 'confusion-matrix-test.py'])
