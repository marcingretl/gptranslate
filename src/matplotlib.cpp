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

#include "gptranslate/backend.hpp"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>
namespace gptranslate {
namespace {
std::string pystr(const std::string& s) {
    std::string o = "'";
    for (char c : s) {
        if (c == '\\' || c == '\'')
            o += '\\';
        if (c == '\n') {
            o += "\\n";
            continue;
        }
        if (c == '\r') {
            o += "\\r";
            continue;
        }
        o += c;
    }
    return o + "'";
}
// Translate gnuplot-specific color names to Matplotlib-compatible names.
// Preserve other names, including hex colors and user-defined RGB values.
std::string mpl_color(const std::string& color) {
    // Normalize gnuplot hexadecimal RGB colors to matplotlib notation.
    std::string hex;

    if (!color.empty() && color[0] == '#') {
        hex = color.substr(1);
    } else if (color.size() > 2 && color[0] == '0' && (color[1] == 'x' || color[1] == 'X')) {
        hex = color.substr(2);
    } else {
        // Translate gnuplot color names not recognized by matplotlib.
        if (color == "dark-green") {
            return "darkgreen";
        }
        return color;
    }

    // Accept hexadecimal RGB values up to six digits.
    if (hex.empty() || hex.size() > 6 ||
        hex.find_first_not_of("0123456789abcdefABCDEF") != std::string::npos) {
        return color;
    }

    // Gnuplot permits omitted leading zeros in RGB values.
    return "#" + std::string(6 - hex.size(), '0') + hex;
}
// Serialize a rectangular numeric matrix as a Python nested list.
std::string matrix_numbers(const ImageMatrix& image) {
    std::ostringstream o;
    o << std::setprecision(17) << '[';
    for (size_t r = 0; r < image.rows.size(); ++r) {
        if (r) {
            o << ',';
        }
        o << '[';
        for (size_t c = 0; c < image.rows[r].size(); ++c) {
            if (c) {
                o << ',';
            }
            // Python requires an explicit representation for nonfinite cells.
            if (std::isnan(image.rows[r][c]))
                o << "float('nan')";
            else
                o << image.rows[r][c];
        }
        o << ']';
    }
    return o.str() + ']';
}
// Emit explicit tic locations, labels, rotation, and outward tick direction.
void emit_tics(std::ostringstream& o,
               const Tics& tics,
               const std::string& axis,
               const std::string& target,
               bool unix_time = false) {
    if (!tics.enabled && tics.format.empty()) {
        return;
    }
    if (!tics.explicit_tics.empty()) {
        o << target << ".set_" << axis << "ticks([";
        for (size_t i = 0; i < tics.explicit_tics.size(); ++i) {
            if (i) {
                o << ",";
            }
            // Preserve the precision of symbolic pi tick positions.
            o << std::setprecision(17) << tics.explicit_tics[i].position;
        }
        o << "], labels=[";
        for (size_t i = 0; i < tics.explicit_tics.size(); ++i) {
            if (i) {
                o << ",";
            }
            o << pystr(tics.explicit_tics[i].label);
        }
        o << "])\n";
    } else if (tics.interval) {
        if (tics.end && tics.start) {
            // A finite gnuplot tic sequence must not extend beyond its end.
            o << target << "." << axis << "axis.set_major_locator(FixedLocator([" << *tics.start
              << " + i * " << *tics.interval << " for i in range(max(0, min(10000, int(("
              << *tics.end << " - " << *tics.start << ") / " << *tics.interval
              << " + 1e-10) + 1)))]))\n";
        } else {
            o << target << "." << axis << "axis.set_major_locator(MultipleLocator("
              << *tics.interval;
            if (tics.start)
                o << ", offset=" << *tics.start;
            o << "))\n";
        }
    }
    if (!tics.format.empty()) {
        if (unix_time) {
            // Keep Unix seconds as axis coordinates, so ranges and explicit tics
            // remain in gnuplot units; only convert values when formatting labels.
            o << target << "." << axis << "axis.set_major_formatter(FuncFormatter("
              << "lambda value, pos: datetime.fromtimestamp(value, "
                 "timezone.utc).strftime("
              << pystr(tics.format) << ")))\n";
        } else {
            o << target << "." << axis << "axis.set_major_formatter(FormatStrFormatter("
              << pystr(tics.format) << "))\n";
        }
    } else if (tics.enabled && tics.explicit_tics.empty() && tics.format.empty() &&
               (tics.major_scale || tics.minor_scale))
        o << target << "." << axis << "axis.set_major_formatter(NullFormatter())\n";
    if (tics.rotation != 0)
        o << "plt.setp(" << target << ".get_" << axis << "ticklabels(), rotation=" << tics.rotation
          << ", ha='right')\n";
    if (tics.font_size)
        o << target << ".tick_params(axis='" << axis << "', labelsize=" << *tics.font_size << ")\n";
    if (tics.direction_explicit) {
        o << target << ".tick_params(axis='" << axis << "', direction='"
          << (tics.outward ? "out" : "in") << "')\n";
    }
    if (tics.major_scale && *tics.major_scale == 0)
        o << target << ".tick_params(axis='" << axis << "', length=0)\n";
}
std::string numbers(const std::vector<Point>& pts, bool x) {
    std::ostringstream o;
    o << std::setprecision(17) << '[';
    for (size_t i = 0; i < pts.size(); ++i) {
        if (i) {
            o << ',';
        }
        o << (x ? pts[i].x : pts[i].y);
    }
    return o.str() + ']';
}
// Add one figure-level hover callback for point-bearing series. The generated
// Python deliberately uses only Matplotlib, without requiring mplcursors.
void emit_point_hover(std::ostringstream& o) {
    o << R"PY(
def _gp_install_hover(figure, artists):
    # Keep each callback's figure and artists local. In gretl4py, successive
    # eval_file() calls may otherwise overwrite shared Python globals.
    annotations = {}

    def _gp_on_hover(event):
        # Inspect only visible point artists in the axes under the mouse.
        # The reverse order gives the topmost (most recently plotted) series priority.
        hit = None
        if event.inaxes is not None:
            for artist in reversed(artists):
                if artist.axes is not event.inaxes or not artist.get_visible():
                    continue
                contains, details = artist.contains(event)
                if not contains:
                    continue
                indices = details.get('ind', ())
                if len(indices) == 0:
                    continue

                # scatter() stores offsets; plot(..., marker='o') stores xy data.
                scatter_artist = isinstance(artist, PathCollection)
                coords = artist.get_offsets() if scatter_artist else artist.get_xydata()
                transform = (artist.get_offset_transform() if scatter_artist
                             else artist.get_transform())
                closest = None
                distance2 = float('inf')
                for index in indices:
                    x_value, y_value = map(float, coords[index])
                    if not (math.isfinite(x_value) and math.isfinite(y_value)):
                        continue
                    pixel_x, pixel_y = transform.transform((x_value, y_value))
                    dist2 = (pixel_x - event.x)**2 + (pixel_y - event.y)**2
                    if dist2 < distance2:
                        closest = (x_value, y_value)
                        distance2 = dist2
                if closest is None:
                    continue

                if not scatter_artist:
                    # Line2D.contains() also accepts positions along line segments.
                    # Show the tooltip only close to an actual marker.
                    radius = max(4.0, artist.get_markersize() / 2 + 2.0) * figure.dpi / 72.0
                    if distance2 > radius**2:
                        continue
                hit = (artist.axes, closest)
                break

        changed = False
        for annotation_ax, annotation in annotations.items():
            if annotation.get_visible() and (hit is None or annotation_ax is not hit[0]):
                annotation.set_visible(False)
                changed = True

        if hit is not None:
            annotation_ax, (x_value, y_value) = hit
            annotation = annotations.get(annotation_ax)
            if annotation is None:
                annotation = annotation_ax.annotate(
                    '', xy=(0, 0), xytext=(10, 10), textcoords='offset points',
                    bbox=dict(boxstyle='round,pad=0.25', fc='white',
                              ec='gray', alpha=0.95),
                    fontsize=9, zorder=10)
                annotation.set_visible(False)
                annotations[annotation_ax] = annotation
            label = f'{x_value:.2f}, {y_value:.2f}'
            if (not annotation.get_visible() or annotation.xy != (x_value, y_value)
                    or annotation.get_text() != label):
                annotation.xy = (x_value, y_value)
                annotation.set_text(label)
                annotation.set_visible(True)
                changed = True

        if changed:
            figure.canvas.draw_idle()

    figure.canvas.mpl_connect('motion_notify_event', _gp_on_hover)
    return annotations

_gp_hover_annotations = _gp_install_hover(fig, _gp_hover_artists)
)PY";
}
} // namespace
std::string MatplotlibBackend::generate(const Document& d) const {
    if (d.plots.empty()) {
        throw std::runtime_error("no plots");
    }
    // Register hover handling only when the plot contains visible point markers.
    // Lines and synthetic reference curves must not trigger a tooltip.
    const bool hover_points = std::any_of(d.plots.begin(), d.plots.end(), [](const Plot& p) {
        return std::any_of(p.series.begin(), p.series.end(), [](const Series& s) {
            return !s.constant_y && !s.analytic_y &&
                   (s.style == Style::Points || s.style == Style::LinesPoints);
        });
    });
    std::ostringstream o;
    if (!backend_name_.empty()) {
        // Select the requested backend before importing pyplot.
        // This affects the generated script, not the translator process.
        if (backend_name_ == "QtAgg") {
            // Prefer PySide6 for the QtAgg backend when it is requested explicitly.
            o << "from PySide6 import QtCore\n";
        }
        o << "import matplotlib\n";
        o << "matplotlib.use(" << pystr(backend_name_) << ")\n";
    }
    o << "import matplotlib.pyplot as plt\n";
    o << "from matplotlib.ticker import MultipleLocator, FixedLocator, "
         "FormatStrFormatter, NullFormatter, "
         "AutoMinorLocator\n";
    o << "from matplotlib.colors import LinearSegmentedColormap, Normalize\n";
    o << "from matplotlib.ticker import FuncFormatter\n";
    o << "from datetime import datetime, timezone\n";
    if (hover_points) {
        // A single registry serves all panels and secondary axes in the figure.
        o << "import math\nfrom matplotlib.collections import PathCollection\n";
        o << "_gp_hover_artists = []\n";
    }
    if (d.positioned_multiplot) {
        // Explicit gnuplot geometry is expressed as fractions of the figure.
        o << "fig = plt.figure(figsize=(" << d.width / 100.0 << "," << d.height / 100.0
          << "), dpi=100)\n";
    } else {
        o << "fig, axes = plt.subplots(" << (d.multiplot ? d.rows : 1) << ", "
          << (d.multiplot ? d.cols : 1) << ", squeeze=False, figsize=(" << d.width / 100.0 << ","
          << d.height / 100.0 << "), dpi=100)\n";
        o << "axes = axes.flatten()\n";
        if (d.plots.size() >
            static_cast<size_t>((d.multiplot ? d.rows : 1) * (d.multiplot ? d.cols : 1))) {
            throw std::runtime_error("more plots than multiplot cells");
        }
    }
    // A multiplot title is a figure-level heading, not the first panel's title.
    if (!d.multiplot_title.empty())
        o << "fig.suptitle(" << pystr(d.multiplot_title) << ")\n";
    for (size_t j = 0; j < d.plots.size(); ++j) {
        const auto& p = d.plots[j];
        if (d.positioned_multiplot) {
            // A gnuplot panel's size and origin describe its layout region, not
            // the axes rectangle. Reserve space inside each region for tick labels,
            // axis titles and the plot title so adjacent panels do not overlap.
            // The outer margins also keep annotations away from the figure edges.
            constexpr double outer_left = 0.04;
            constexpr double outer_bottom = 0.04;
            constexpr double outer_width = 0.92;
            constexpr double outer_height = 0.92;
            constexpr double inset_left = 0.10;
            constexpr double inset_bottom = 0.09;
            constexpr double inset_width = 0.82;
            constexpr double inset_height = 0.78;
            const double left = outer_left + outer_width * (p.origin_x + inset_left * p.size_x);
            const double bottom =
                outer_bottom + outer_height * (p.origin_y + inset_bottom * p.size_y);
            const double width = outer_width * inset_width * p.size_x;
            const double height = outer_height * inset_height * p.size_y;
            o << "ax = fig.add_axes([" << left << "," << bottom << "," << width << "," << height
              << "])\n";
        } else {
            // A skipped gridplot cell has no Plot, so use its explicit index.
            const auto cells =
                static_cast<size_t>((d.multiplot ? d.rows : 1) * (d.multiplot ? d.cols : 1));
            if (p.panel_index >= cells)
                throw std::runtime_error("more plots than multiplot cells");
            o << "ax = axes[" << p.panel_index << "]\n";
        }
        bool twin = false;
        // Gnuplot matrix coordinates start at (0, 0) in the bottom-left.
        // Nearest-neighbor interpolation preserves individual matrix cells.
        // A defined palette is normalized over its own breakpoint domain.
        // Color-axis limits are applied independently to the image normalization.
        if (!p.colors.stops.empty()) {
            const auto& stops = p.colors.stops;
            const double low = stops.front().value, high = stops.back().value;
            o << "cmap = LinearSegmentedColormap.from_list('gnuplot_palette', [";
            o << std::setprecision(17);
            for (size_t k = 0; k < stops.size(); ++k) {
                if (k) {
                    o << ", ";
                }
                o << "(" << (stops[k].value - low) / (high - low) << ", ";
                if (stops[k].rgb) {
                    const auto& rgb = *stops[k].rgb;
                    o << "(" << rgb[0] << ", " << rgb[1] << ", " << rgb[2] << ")";
                } else {
                    o << pystr(mpl_color(stops[k].color));
                }
                o << ")";
            }
            o << "])\n";
        }
        for (const auto& image : p.images) {
            o << "ax.imshow(" << matrix_numbers(image)
              << ", origin='lower', interpolation='nearest', aspect='auto'";
            if (!p.colors.stops.empty()) {
                o << ", cmap=cmap";
            }
            if (p.colors.min || p.colors.max) {
                // A missing bound uses the observed image range, as with autoscaling.
                o << ", norm=Normalize(";
                o << "vmin=" << (p.colors.min ? std::to_string(*p.colors.min) : "None");
                o << ", vmax=" << (p.colors.max ? std::to_string(*p.colors.max) : "None") << ")";
            }
            o << ")\n";
            // Colorboxes are optional; the most recently rendered image owns the
            // scale.
            if (p.colorbox) {
                o << "_gp_colorbar = fig.colorbar(ax.images[-1], ax=ax)\n";
                if (!p.colors.label.empty())
                    o << "_gp_colorbar.set_label(" << pystr(p.colors.label) << ")\n";
            }
        }
        // Place matrix labels at their gnuplot cell coordinates (column, row).
        // The numeric format and RGB condition are retained as plot data.
        for (const auto& labels : p.matrix_labels) {
            o << "for row_index, values in enumerate(" << matrix_numbers(labels.matrix) << "):\n";
            o << "    for column_index, value in enumerate(values):\n";
            // Suppress missing-cell labels in triangular correlation plots.
            o << "        if value != value:\n";
            o << "            continue\n";
            o << "        color = '#" << labels.below_color << "' if value " << labels.comparison
              << " " << std::setprecision(17) << labels.threshold << " else '#"
              << labels.otherwise_color << "'\n";
            o << "        ax.text(column_index, row_index, " << pystr(labels.format)
              << " % value, color=color, ha='center', va='center')\n";
        }
        // Analytic functions may precede inline data in a single gnuplot plot
        // command. Determine their x domain from all plotted data series rather
        // than from Matplotlib's initial (0, 1) axes limits.
        std::optional<double> data_x_min, data_x_max;
        for (const auto& source : p.series) {
            for (const auto& point : source.points) {
                if (!std::isfinite(point.x))
                    continue;
                data_x_min = data_x_min ? std::min(*data_x_min, point.x) : point.x;
                data_x_max = data_x_max ? std::max(*data_x_max, point.x) : point.x;
            }
        }
        // In auto-contrast mode, distinguish graphical roles in combined plots.
        // Single-series plots retain their original source colors.
        // Colors are assigned by role and occurrence, not by the source's palette.
        size_t line_index = 0, box_index = 0, other_index = 0;
        const bool contrast_plot = auto_contrast_ && p.series.size() > 1;
        // Keep error bars the same color as the preceding point-estimate line.
        std::string last_line_color;
        // Separate positive and negative stacking bases for histogram series.
        o << "_hist_positive = {}\n_hist_negative = {}\n";
        for (const auto& s : p.series) {
            if (s.yaxis == 2 && !twin) {
                o << "ax2 = ax.twinx()\n";
                twin = true;
            }
            o << "target = " << (s.yaxis == 2 ? "ax2" : "ax") << "\n";
            o << "x = " << numbers(s.points, true) << "\ny = " << numbers(s.points, false) << "\n";
            if (p.polar && !s.constant_y && !s.analytic_y) {
                // Convert gnuplot theta/r data to Cartesian coordinates on the
                // ordinary Matplotlib axes (as used by libgretl's root plots).
                o << "import math\n";
                o << "x, y = ([radius * math.cos(theta) for theta, radius in zip(x, y)], "
                     "[radius * math.sin(theta) for theta, radius in zip(x, y)])\n";
            }
            o << "kw = {'label': " << (s.title.empty() ? "'_nolegend_'" : pystr(s.title)) << "}";
            // All layers of a libgretl boxplot share one color, including
            // the median and the two dotted notch boundaries.
            if (s.style == Style::Candlesticks) {
                o << "\nkw['color'] = '#E76F00'";
            } else if (s.style == Style::ErrorBars && !s.explicit_color &&
                       !last_line_color.empty()) {
                // libgretl emits each IRF line immediately before its error bars.
                o << "\nkw['color'] = " << pystr(last_line_color);
            } else if (contrast_plot && !s.explicit_color && s.style != Style::FilledCurve) {
                // Each role has its own palette; repeated series cycle through distinct
                // hues.
                static const char* box_colors[] = {"#B8C5D5", "#86A7C1", "#D6B68C", "#A8BFA3"};
                static const char* line_colors[] = {"#C05A35", "#238477", "#694FA3", "#A34172"};
                static const char* other_colors[] = {"#238477", "#694FA3", "#C05A35", "#A34172"};
                const char* color;
                if (s.style == Style::Boxes) {
                    color = box_colors[(box_index++) % 4];
                } else if (s.style == Style::Lines || s.style == Style::LinesPoints ||
                           s.style == Style::Steps)
                    color = line_colors[(line_index++) % 4];
                else
                    color = other_colors[(other_index++) % 4];
                o << "\nkw['color'] = " << pystr(color);
                if (s.style == Style::Lines)
                    last_line_color = color;
            } else if (!s.color.empty()) {
                const auto color = mpl_color(s.color);
                o << "\nkw['color'] = " << pystr(color);
                if (s.style == Style::Lines)
                    last_line_color = color;
            }
            o << "\nkw['linewidth'] = " << s.width << "\n";
            // Translate the gnuplot dash type into a matplotlib line style.
            if (s.dash_type == 2) {
                o << "kw['linestyle'] = '--'\n";
            } else if (s.dash_type == 3) {
                // Gnuplot's line type 0 uses a dotted reference line.
                o << "kw['linestyle'] = ':'\n";
            }
            if (s.analytic_y) {
                // The analytic-function translator currently uses Cartesian x.
                // Never silently misrepresent a polar function as y = f(x).
                if (p.polar)
                    throw std::runtime_error("polar analytic functions are not supported");
                // Sample the validated function over the explicit x range, or
                // over the full data x range when functions precede data series.
                // math functions are evaluated pointwise; invalid domain points become
                // NaN.
                if (s.style != Style::Lines) {
                    throw std::runtime_error("unsupported analytic function style");
                }
                o << "x_min, x_max = ";
                if (p.x.min) {
                    o << std::setprecision(17) << *p.x.min;
                } else if (data_x_min) {
                    o << std::setprecision(17) << *data_x_min;
                } else {
                    o << "ax.get_xlim()[0]";
                }
                o << ", ";
                if (p.x.max) {
                    o << std::setprecision(17) << *p.x.max;
                } else if (data_x_max) {
                    o << std::setprecision(17) << *data_x_max;
                } else {
                    o << "ax.get_xlim()[1]";
                }
                o << "\n";
                o << "x = [x_min + (x_max - x_min)*i/199 for i in range(200)]\n";
                o << "import math\n";
                o << "def _gp_fit(value):\n";
                o << "    try: return " << *s.analytic_y << "\n";
                o << "    except (ValueError, ZeroDivisionError, OverflowError): "
                     "return "
                     "float('nan')\n";
                o << "y = [_gp_fit(value) for value in x]\n";
                o << "target.plot(x, y, zorder=3, **kw)\n";
                continue;
            }
            if (s.constant_y) {
                // A constant gnuplot function spans the current axes independently of x
                // data. axhline uses axis-relative horizontal coordinates and preserves
                // line styles.
                if (s.style != Style::Lines) {
                    throw std::runtime_error("unsupported constant function style");
                }
                if (p.polar) {
                    // In polar mode `plot 1` means r=1: a unit circle, not
                    // a horizontal line. Closing the sampled curve avoids a gap.
                    o << "import math\n";
                    o << "_gp_angles = [2 * math.pi * i / 360 for i in range(361)]\n";
                    o << "target.plot([" << std::setprecision(17) << *s.constant_y
                      << " * math.cos(theta) for theta in _gp_angles], [" << *s.constant_y
                      << " * math.sin(theta) for theta in _gp_angles], zorder=3, **kw)\n";
                } else {
                    o << "target.axhline(" << std::setprecision(17) << *s.constant_y << ", **kw)\n";
                }
                continue;
            }
            switch (s.style) {
                case Style::Candlesticks: {
                    // Draw the gnuplot five-column candlestick geometry directly.
                    // Equal open/close values produce horizontal whisker caps.
                    o << "_low = [";
                    for (size_t j = 0; j < s.candle_low.size(); ++j) {
                        if (j)
                            o << ", ";
                        o << std::setprecision(17) << s.candle_low[j];
                    }
                    o << "]\n_high = [";
                    for (size_t j = 0; j < s.candle_high.size(); ++j) {
                        if (j)
                            o << ", ";
                        o << std::setprecision(17) << s.candle_high[j];
                    }
                    o << "]\n_top = [";
                    for (size_t j = 0; j < s.candle_top.size(); ++j) {
                        if (j)
                            o << ", ";
                        o << std::setprecision(17) << s.candle_top[j];
                    }
                    o << "]\n";
                    o << "_cw = " << (p.box_width ? *p.box_width : 0.5) << "\n";
                    // Gnuplot lt 0 is used for dotted notch bounds in libgretl.
                    o << "_cstyle = " << (s.line_type == 0 ? "':'" : "'-'") << "\n";
                    o << "for _cx, _a, _lo, _hi, _b in zip(x, y, _low, _high, _top):\n";
                    o << "    target.vlines(_cx, _lo, _hi, color=kw.get('color', 'black'), "
                         "linewidth=kw['linewidth'], linestyles=_cstyle)\n";
                    o << "    if _a == _b:\n";
                    o << "        target.hlines(_a, _cx-_cw/2, _cx+_cw/2, "
                         "color=kw.get('color', "
                         "'black'), linewidth=kw['linewidth'], linestyles=_cstyle)\n";
                    o << "    else:\n";
                    o << "        from matplotlib.patches import Rectangle\n";
                    o << "        target.add_patch(Rectangle((_cx-_cw/2, min(_a,_b)), _cw, "
                         "abs(_b-_a), fill=False, edgecolor=kw.get('color', 'black'), "
                         "linewidth=kw['linewidth'], linestyle=_cstyle))\n";
                    break;
                }
                case Style::FilledCurve:
                    // A three-column filledcurve defines lower and upper boundaries.
                    o << "upper = [";
                    for (size_t j = 0; j < s.upper.size(); ++j) {
                        if (j)
                            o << ", ";
                        o << std::setprecision(17) << s.upper[j];
                    }
                    o << "]\n";
                    o << "kw.pop('linewidth', None)\n";
                    // Apply the global gnuplot fill density to confidence bands.
                    o << "target.fill_between(x, y, upper, alpha=" << p.fill_alpha
                      << ", linewidth=0, zorder=0, **kw)\n";
                    break;
                case Style::ErrorBars: {
                    // Matplotlib requires nonnegative errors relative to the central
                    // value. gretl's bootstrap quantile interval can exclude the
                    // point estimate, so use the interval midpoint for the whiskers,
                    // then draw the actual estimate as a separate point marker.
                    o << "lower = [";
                    for (size_t j = 0; j < s.lower.size(); ++j) {
                        if (j)
                            o << ", ";
                        o << std::setprecision(17) << s.lower[j];
                    }
                    o << "]\nupper = [";
                    for (size_t j = 0; j < s.upper.size(); ++j) {
                        if (j)
                            o << ", ";
                        o << std::setprecision(17) << s.upper[j];
                    }
                    o << "]\n";
                    o << "_mid = [(lo + hi) / 2 for lo, hi in zip(lower, upper)]\n";
                    o << "_yerr = [[mid - lo for mid, lo in zip(_mid, lower)], "
                         "[hi - mid for mid, hi in zip(_mid, upper)]]\n";
                    o << "target.errorbar(x, _mid, yerr=_yerr, fmt='none', "
                         "ecolor=kw.get('color', 'C0'), elinewidth=kw['linewidth'], "
                         "capsize=2, label=kw['label'], zorder=2)\n";
                    // Use a distinct marker collection to preserve the actual
                    // estimate, which is not necessarily the midpoint.
                    o << "target.scatter(x, y, s=12, color=kw.get('color', 'C0'), "
                         "zorder=3)\n";
                    break;
                }
                case Style::Lines:
                    o << "target.plot(x, y, zorder=3, **kw)\n";
                    break;
                case Style::Points:
                    // Store the actual Matplotlib artist, so hover labels use the
                    // final coordinates (including the polar-to-Cartesian transform).
                    o << "_gp_hover_artist = target.scatter(x, y, s=" << std::setprecision(17)
                      << (18 * s.point_size * s.point_size) << ", **kw)\n";
                    o << "_gp_hover_artists.append(_gp_hover_artist)\n";
                    break;
                case Style::Dots:
                    o << "target.scatter(x, y, s=" << std::setprecision(17)
                      << (4 * s.point_size * s.point_size) << ", **kw)\n";
                    break;
                case Style::LinesPoints:
                    o << "_gp_hover_artist, = target.plot(x, y, marker='o', markersize="
                      << std::setprecision(17) << (6 * s.point_size) << ", zorder=3, **kw)\n";
                    o << "_gp_hover_artists.append(_gp_hover_artist)\n";
                    break;
                case Style::Impulses:
                    o << "target.vlines(x, 0, y, **kw)\n";
                    break;
                case Style::Steps:
                    o << "target.step(x, y, where='post', **kw)\n";
                    break;
                case Style::Histograms: {
                    // Rowstacked histograms accumulate contributions at each row index.
                    o << "kw.pop('linewidth', None)\n";
                    if (p.histogram_rowstacked) {
                        o << "_hist_bottom = [_hist_positive.get(v, 0.0) if h >= 0 "
                             "else _hist_negative.get(v, 0.0) for v, h in zip(x, y)]\n";
                        o << "target.bar(x, y, bottom=_hist_bottom, width=0.8, alpha="
                          << p.fill_alpha << ", zorder=1, **kw)\n";
                        o << "for _hx, _hy, _hb in zip(x, y, _hist_bottom):\n";
                        o << "    if _hy >= 0: _hist_positive[_hx] = _hb + _hy\n";
                        o << "    else: _hist_negative[_hx] = _hb + _hy\n";
                    } else {
                        o << "target.bar(x, y, width=0.8, alpha=" << p.fill_alpha
                          << ", zorder=1, **kw)\n";
                    }
                    break;
                }
                case Style::Boxes: {
                    // Matplotlib bar widths are always in x-axis units. Gnuplot's default
                    // derives its width from point spacing, not from a fixed value of 0.8.
                    o << "_box_steps = [abs(x[i+1] - x[i]) for i in range(len(x)-1) if "
                         "x[i+1] != "
                         "x[i]]\n";
                    o << "_box_spacing = min(_box_steps) if _box_steps else 1.0\n";
                    if (p.box_width) {
                        o << "_box_width = " << std::setprecision(17) << *p.box_width;
                        if (p.box_width_relative) {
                            o << " * _box_spacing";
                        }
                        o << "\n";
                    } else {
                        o << "_box_width = 0.8 * _box_spacing\n";
                    }
                    if (!p.box_width) {
                        // Dense step-sampled histograms (as emitted by PARMA) repeat the same
                        // bin height at many consecutive x positions. Merge those plateaus
                        // into rectangles, while preserving ordinary histograms
                        // point-for-point.
                        o << "_box_centers, _box_heights, _box_widths = [], [], []\n";
                        o << "_box_i = 0\n";
                        o << "while _box_i < len(x):\n";
                        o << "    _box_j = _box_i + 1\n";
                        o << "    while _box_j < len(x) and y[_box_j] == y[_box_i] "
                             "and abs((x[_box_j] - x[_box_j-1]) - _box_spacing) <= "
                             "max(1e-3 * _box_spacing, 1e-12):\n";
                        o << "        _box_j += 1\n";
                        o << "    if _box_j - _box_i >= 3:\n";
                        o << "        if y[_box_i] != 0:\n";
                        o << "            _box_centers.append((x[_box_i] + x[_box_j-1]) / "
                             "2)\n";
                        o << "            _box_heights.append(y[_box_i])\n";
                        o << "            _box_widths.append(x[_box_j-1] - x[_box_i] + "
                             "_box_spacing)\n";
                        o << "    else:\n";
                        o << "        for _box_k in range(_box_i, _box_j):\n";
                        o << "            if y[_box_k] != 0:\n";
                        o << "                _box_centers.append(x[_box_k])\n";
                        o << "                _box_heights.append(y[_box_k])\n";
                        o << "                _box_widths.append(_box_width)\n";
                        o << "    _box_i = _box_j\n";
                        o << "target.bar(_box_centers, _box_heights, width=_box_widths, "
                             "alpha="
                          << p.fill_alpha << ", zorder=1, **kw)\n";
                    } else {
                        // Explicit gnuplot box widths must not be inferred from repeated
                        // values.
                        o << "target.bar(x, y, width=_box_width, alpha=" << p.fill_alpha
                          << ", zorder=1, **kw)\n";
                    }
                    break;
                }
            }
        }
        // Match gnuplot's border bitmask (bottom=1, left=2, top=4, right=8).
        o << "for side, bit in [('bottom',1),('left',2),('top',4),('right',8)]:\n";
        o << "    ax.spines[side].set_visible(bool(" << p.border << " & bit))\n";
        if (!p.border_color.empty()) {
            o << "    ax.spines[side].set_color(" << pystr(p.border_color) << ")\n";
        }
        if (!p.mirror_tics || !p.y.tics.mirror) {
            o << "ax.tick_params(top=False, right=False)\n";
        }
        o << "ax.set_title(" << pystr(p.title) << ")\nax.set_xlabel(" << pystr(p.x.label)
          << ")\nax.set_ylabel(" << pystr(p.y.label) << ")\n";
        if (twin) {
            o << "ax2.set_ylabel(" << pystr(p.y2.label) << ")\n";
        }
        if (p.x.min) {
            o << "ax.set_xlim(left=" << *p.x.min << ")\n";
        }
        if (p.x.max) {
            o << "ax.set_xlim(right=" << *p.x.max << ")\n";
        }
        if (p.y.min) {
            o << "ax.set_ylim(bottom=" << *p.y.min << ")\n";
        }
        if (p.y.max) {
            o << "ax.set_ylim(top=" << *p.y.max << ")\n";
        }
        if (p.x.log) {
            o << "ax.set_xscale('log', base=" << p.x.log_base << ")\n";
        }
        if (p.y.log) {
            o << "ax.set_yscale('log', base=" << p.y.log_base << ")\n";
        }
        if (twin) {
            if (p.y2.min) {
                o << "ax2.set_ylim(bottom=" << *p.y2.min << ")\n";
            }
            if (p.y2.max) {
                o << "ax2.set_ylim(top=" << *p.y2.max << ")\n";
            }
            if (p.y2.log) {
                o << "ax2.set_yscale('log', base=" << p.y2.log_base << ")\n";
            }
        }
        if (p.axis_offsets) {
            // Bare numeric offsets add to the data limits, not to fractions
            // of the axes width or height.
            const auto& offsets = *p.axis_offsets;
            o << "_gp_x0, _gp_x1 = ax.dataLim.intervalx\n";
            o << "_gp_y0, _gp_y1 = ax.dataLim.intervaly\n";
            if (!p.x.min && !p.x.max && !p.x.log)
                o << "ax.set_xlim(_gp_x0 - " << offsets[0] << ", _gp_x1 + " << offsets[1] << ")\n";
            if (!p.y.min && !p.y.max && !p.y.log)
                o << "ax.set_ylim(_gp_y0 - " << offsets[3] << ", _gp_y1 + " << offsets[2] << ")\n";
        }
        if (p.graph_offsets) {
            // Convert graph-relative fractions to data-coordinate extensions.
            // Unlike symmetric ax.margins(), this preserves independent sides.
            const auto& offsets = *p.graph_offsets;
            o << "_gp_x0, _gp_x1 = ax.dataLim.intervalx\n";
            o << "_gp_y0, _gp_y1 = ax.dataLim.intervaly\n";
            if (!p.x.min && !p.x.max && !p.x.log) {
                o << "if _gp_x1 > _gp_x0:\n";
                o << "    _gp_span = (_gp_x1 - _gp_x0) / " << (1.0 - offsets[0] - offsets[1])
                  << "\n";
                o << "    ax.set_xlim(_gp_x0 - " << offsets[0] << " * _gp_span, _gp_x1 + "
                  << offsets[1] << " * _gp_span)\n";
            }
            if (!p.y.min && !p.y.max && !p.y.log) {
                o << "if _gp_y1 > _gp_y0:\n";
                o << "    _gp_span = (_gp_y1 - _gp_y0) / " << (1.0 - offsets[2] - offsets[3])
                  << "\n";
                o << "    ax.set_ylim(_gp_y0 - " << offsets[3] << " * _gp_span, _gp_y1 + "
                  << offsets[2] << " * _gp_span)\n";
            }
        }
        if (p.xzeroaxis) {
            // A zero reference axis must not enlarge the data-driven y range.
            // Explicit gnuplot yrange bounds have already been applied above.
            o << "_gp_ymin, _gp_ymax = ax.get_ylim()\n";
            o << "if _gp_ymin <= 0 <= _gp_ymax:\n";
            o << "    ax.axhline(0, color='gray', linewidth=0.7, zorder=0)\n";
            o << "    ax.set_ylim(_gp_ymin, _gp_ymax)\n";
        }
        if (p.yzeroaxis) {
            // Likewise the vertical x=0 reference line must not change x limits.
            o << "_gp_xmin, _gp_xmax = ax.get_xlim()\n";
            o << "if _gp_xmin <= 0 <= _gp_xmax:\n";
            o << "    ax.axvline(0, color='gray', linewidth=0.7, zorder=0)\n";
            o << "    ax.set_xlim(_gp_xmin, _gp_xmax)\n";
        }
        if (p.equal_units) {
            // A negative gnuplot ratio of -1 preserves equal lengths per data
            // unit while allowing a non-square axes rectangle.
            o << "ax.set_aspect('equal', adjustable='box')\n";
        }
        if (p.square) {
            // Keep the axes box square and Cartesian units equally scaled, even
            // when the document's terminal or multiplot panel is rectangular.
            o << "ax.set_box_aspect(1)\n";
            o << "ax.set_aspect('equal', adjustable='datalim')\n";
        }
        // Gnuplot labels may use either data or axes-relative graph coordinates.
        for (const auto& label : p.labels) {
            if (label.point_marker) {
                // Confidence ellipses mark their estimated coefficient pair.
                std::string color = "C1";
                if (label.point_line_type > 0 &&
                    static_cast<size_t>(label.point_line_type) <= d.linetype_colors.size() &&
                    !d.linetype_colors[label.point_line_type - 1].empty()) {
                    color = mpl_color(d.linetype_colors[label.point_line_type - 1]);
                }
                if (!label.point_color.empty())
                    color = mpl_color(label.point_color);
                const std::string marker = label.point_type == 7 ? "o" : "+";
                o << "ax.plot([" << std::setprecision(17) << label.x << "], [" << label.y
                  << "], marker=" << pystr(marker)
                  << ", linestyle='None', markersize=" << (8 * label.point_size)
                  << ", color=" << pystr(color) << ", zorder=5)\n";
                if (label.text.empty())
                    continue;
                // Gnuplot's offset units are character cells, approximately
                // six points wide/tall at the default font size.
                o << "ax.annotate(" << pystr(label.text) << ", (" << label.x << ", " << label.y
                  << "), xytext=(" << label.offset_x * 6.0 << ", " << label.offset_y * 6.0
                  << "), textcoords='offset points', ha='left', va='bottom')\n";
                continue;
            }
            o << "ax.text(" << std::setprecision(17) << label.x << ", " << label.y << ", "
              << pystr(label.text) << ", ha='" << (label.centered ? "center" : "left")
              << "', va='center'";
            if (label.graph_coordinates)
                o << ", transform=ax.transAxes";
            o << ")\n";
        }
        // Apply named ticks to primary axes. Secondary x/y tic axes are
        // independent.
        emit_tics(o, p.x.tics, "x", "ax", p.xdata_time && p.timefmt == "%s");
        emit_tics(o, p.y.tics, "y", "ax");
        if (p.mxtics > 1) {
            o << "ax.xaxis.set_minor_locator(AutoMinorLocator(" << p.mxtics << "))\n";
        }
        // Unsetting tics wins over any earlier explicit interval or formatter.
        if (p.xtics_disabled)
            o << "ax.set_xticks([])\n";
        if (p.ytics_disabled)
            o << "ax.set_yticks([])\n";
        if (p.x2.tics.enabled || !p.x2.label.empty() || p.x2.min || p.x2.max) {
            // A specified x2range uses an independent coordinate system. In the
            // residual periodogram, for example, x2 spans four times the x range.
            // A plain secondary_xaxis would incorrectly interpret its tic values
            // in primary-axis coordinates, so use a shared-position twiny axis.
            if (p.x2.min || p.x2.max) {
                o << "top_ax = ax.twiny()\n";
                if (p.x2.min) {
                    o << "top_ax.set_xlim(left=" << *p.x2.min << ")\n";
                }
                if (p.x2.max) {
                    o << "top_ax.set_xlim(right=" << *p.x2.max << ")\n";
                }
            } else {
                // Keep the existing identity mapping when no independent range
                // is given (e.g. linked axes used by libgretl matrix plots).
                o << "top_ax = ax.secondary_xaxis('top')\n";
            }
            o << "top_ax.set_xlabel(" << pystr(p.x2.label) << ")\n";
            if (p.x2.tics.enabled) {
                emit_tics(o, p.x2.tics, "x", "top_ax");
                if (p.mx2tics > 1) {
                    o << "top_ax.xaxis.set_minor_locator(AutoMinorLocator(" << p.mx2tics << "))\n";
                }
            } else {
                // Gnuplot does not enable secondary tics merely by giving a
                // label or setting a numeric range.
                o << "top_ax.set_xticks([])\n";
            }
        }
        if (p.y2.tics.enabled && !twin) {
            o << "right_ax = ax.secondary_yaxis('right')\n";
            emit_tics(o, p.y2.tics, "y", "right_ax");
            if (p.my2tics > 1) {
                o << "right_ax.yaxis.set_minor_locator(AutoMinorLocator(" << p.my2tics << "))\n";
            }
        } else if (p.y2.tics.enabled) {
            emit_tics(o, p.y2.tics, "y", "ax2");
        }
        if (p.grid_x) {
            o << "ax.grid(True, axis='x')\n";
        }
        if (p.grid_y) {
            o << "ax.grid(True, axis='y')\n";
        }
        if (p.legend) {
            o << "handles, labels = ax.get_legend_handles_labels()\n";
            if (twin) {
                o << "h2, l2 = ax2.get_legend_handles_labels()\nhandles += h2\nlabels "
                     "+= l2\n";
            }
            if (p.legend_outside) {
                // Anchor the legend outside the axes according to gnuplot key options.
                const auto& h = p.legend_horizontal;
                const auto& v = p.legend_vertical;
                const std::string loc = h == "center"
                                          ? (v == "top" ? "lower center" : "upper center")
                                          : std::string(v == "top"      ? "upper "
                                                        : v == "bottom" ? "lower "
                                                                        : "center ") +
                                                (h == "left" ? "right" : "left");
                const std::string x = h == "left" ? "-0.02" : h == "right" ? "1.02" : "0.5";
                const std::string y = v == "top" ? "1.02" : v == "bottom" ? "-0.02" : "0.5";
                // Long vertical outside legends need multiple columns and more figure
                // width. Compute the column count at runtime from the actual figure
                // height.
                if (h == "left" || h == "right") {
                    o << "if handles:\n";
                    o << "    _key_rows = max(1, int(fig.get_size_inches()[1] * 72 / "
                         "15))\n";
                    o << "    _key_cols = max(1, (len(handles) + _key_rows - 1) // "
                         "_key_rows)\n";
                    o << "    if _key_cols > 1:\n";
                    o << "        _key_width, _key_height = fig.get_size_inches()\n";
                    o << "        fig.set_size_inches(_key_width + 1.0 * (_key_cols - "
                         "1), "
                         "_key_height, forward=True)\n";
                    o << "    ax.legend(handles, labels, loc=" << pystr(loc) << ", bbox_to_anchor=("
                      << x << ", " << y << "), ncol=_key_cols)\n";
                } else {
                    o << "if handles: ax.legend(handles, labels, loc=" << pystr(loc)
                      << ", bbox_to_anchor=(" << x << ", " << y << "))\n";
                }
            } else {
                o << "if handles: ax.legend(handles, labels"
                  << (p.legend_location.empty() ? "" : ", loc=" + pystr(p.legend_location))
                  << ")\n";
            }
        }
    }
    if (!d.positioned_multiplot) {
        // Keep explicitly skipped cells invisible, including holes in the grid.
        o << "for _gp_index, _gp_ax in enumerate(axes):\n";
        o << "    if _gp_index not in {";
        for (size_t j = 0; j < d.plots.size(); ++j) {
            if (j)
                o << ", ";
            o << d.plots[j].panel_index;
        }
        o << "}: _gp_ax.set_visible(False)\n";
        // Reserve space for a multiplot heading when one is present.
        // tight_layout also accounts for legends attached outside the axes.
        o << (d.multiplot_title.empty() ? "fig.tight_layout()\n"
                                        : "fig.tight_layout(rect=(0, 0, 1, 0.94))\n");
    }
    if (hover_points)
        emit_point_hover(o);
    // Generated scripts are display-neutral by default; --add-show explicitly
    // requests an interactive display statement at the end of the script.
    if (show_) {
        o << "plt.show()\n";
    }
    return o.str();
}
} // namespace gptranslate
