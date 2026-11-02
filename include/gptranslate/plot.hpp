/*
 * gptranslate -- gnuplot-to-Matplotlib translator for gretl and gretl4py
 * Copyright (C) 2026 Marcin Błażejowski
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once
#include <map>
#include <optional>
#include <string>
#include <vector>
namespace gptranslate {
struct Point {
    double x{}, y{};
};
enum class Style {
    Lines,
    Points,
    LinesPoints,
    Impulses,
    Steps,
    Boxes,
    Histograms,
    Dots,
    FilledCurve,
    ErrorBars,
    Candlesticks
};
struct Series {
    std::string title, color;
    // Distinguish an explicitly selected plot color or line type from palette
    // fallback.
    bool explicit_color{false};
    Style style{Style::Lines};

    // Gnuplot dash type: 1 = solid, 2 = dashed.
    int dash_type{1};
    // Original gnuplot line type; zero marks the dotted boxplot notch bounds.
    int line_type{-1};
    int yaxis{1};
    double width{1.5};
    // Constant functions are independent of inline or named data sources.
    std::optional<double> constant_y;
    // Affine function y = intercept + slope*x, independent of data rows.
    std::optional<std::pair<double, double>> linear_y;
    // Validated gnuplot analytic expression translated into Python syntax.
    std::optional<std::string> analytic_y;
    std::vector<Point> points;
    // Error bars store both bounds independently of their point estimates.
    std::vector<double> lower;
    // Upper boundary values for filled curves and error bars, aligned with points.
    std::vector<double> upper;
    // Candlestick columns: x, box bottom, whisker bottom, whisker top, box top.
    std::vector<double> candle_low, candle_high, candle_top;
};
struct Tic {
    std::string label;
    double position{};
};
struct Tics {
    bool enabled{false};
    std::vector<Tic> explicit_tics;
    std::optional<double> interval;
    std::optional<double> start,
        end; // Optional gnuplot start,incr[,end] tic sequence.
    std::string format;
    std::optional<double> major_scale, minor_scale;
    double rotation{0};
    bool outward{false};
    bool mirror{true};
};
struct Axis {
    std::string label;
    std::optional<double> min, max;
    bool log{false};
    double log_base{10.0};
    Tics tics;
};
// Rectangular numeric grid in gnuplot matrix row order (bottom row first).
struct ImageMatrix {
    std::vector<std::vector<double>> rows;
};
// Text labels attached to numeric matrix cells. The coordinates and values
// follow gnuplot's matrix pseudo-columns ($1, $2, $3).
struct MatrixLabels {
    ImageMatrix matrix;
    std::string format;
    double threshold{};
    std::string comparison;
    std::string below_color, otherwise_color;
};
// Gnuplot color-axis range and piecewise-defined color palette.
struct PaletteStop {
    double value{};
    std::string color;
};
struct ColorScale {
    std::optional<double> min, max;
    std::vector<PaletteStop> stops;
};
struct Plot {
    struct Label {
        std::string text;
        double x{}, y{};
        // Graph coordinates are relative to the axes rectangle, not data values.
        bool graph_coordinates{false};
        // Gnuplot defaults to left alignment unless `center` is requested.
        bool centered{false};
    };
    std::vector<Label> labels;
    bool xtics_disabled{false}, ytics_disabled{false};
    // Polar data columns are angle (radians) and radius, not Cartesian x/y.
    bool polar{false};
    // `set size square` fixes the axes geometry, unlike a multiplot panel size.
    bool square{false};
    // Gnuplot multiplot placement, normalized to the figure dimensions.
    double origin_x{0.0}, origin_y{0.0};
    double size_x{1.0}, size_y{1.0};
    std::string title;
    Axis x, y, x2, y2;
    // Preserve gnuplot time-axis settings for backend conversion.
    bool xdata_time{false};
    std::string timefmt;
    int mx2tics{0}, my2tics{0};
    bool autoscale_fix{false}, link_x{false}, link_y{false};
    bool grid_x{false}, grid_y{false}, legend{true};
    std::string legend_location;
    // Placement outside the axes and requested horizontal/vertical alignment.
    bool legend_outside{false};
    std::string legend_horizontal{"right"}, legend_vertical{"center"};
    // Gnuplot colorbox visibility applies to palette-based matrix images.
    bool colorbox{true};
    bool mirror_tics{true}, xzeroaxis{false}, yzeroaxis{false};
    // Approximate gnuplot solid fill density for box-style plots.
    double fill_alpha{1.0};
    // Gnuplot rowstacked histograms accumulate each row across series.
    bool histogram_rowstacked{false};
    // Optional gnuplot box width: relative to point spacing or absolute x units.
    std::optional<double> box_width;
    bool box_width_relative{false};
    int border{15};
    std::string border_color;
    std::vector<Series> series;
    std::vector<ImageMatrix> images;
    std::vector<MatrixLabels> matrix_labels;
    ColorScale colors;
};
struct Document {
    // Named gnuplot data blocks, preserving their original rows.
    std::map<std::string, std::vector<std::string>> data_blocks;
    std::vector<Plot> plots;
    std::vector<std::string> linetype_colors;
    std::string output, terminal{"pngcairo"};
    int width{800}, height{600};
    bool multiplot{false};
    // True when panels are positioned explicitly using size/origin.
    bool positioned_multiplot{false};
    int rows{1}, cols{1};
};
struct Diagnostic {
    size_t line{};
    std::string message;
};
struct ParseResult {
    Document document;
    std::vector<Diagnostic> errors, warnings;
};
} // namespace gptranslate
