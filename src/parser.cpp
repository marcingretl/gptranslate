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

#include "gptranslate/parser.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <limits>
#include <map>
#include <regex>
#include <sstream>
#include <stdexcept>
namespace gptranslate {
namespace {
// Translate a restricted arithmetic expression without executing source text.
// The lexer accepts numbers, x, assigned scalars, supported math functions,
// the constant pi, and arithmetic only.
// Invalid expressions are rejected instead of being silently reduced to a
// constant.
std::optional<std::string> analytic_expression(const std::string& input,
                                               const std::map<std::string, double>& variables) {
    static const std::regex token(
        R"((?:[0-9]+(?:[.,][0-9]+)?|[.,][0-9]+)(?:[eE][+-]?[0-9]+)?|[A-Za-z_][A-Za-z_0-9]*|\*\*|[+*/()\-])");
    std::string result;
    bool contains_x = false;
    int depth = 0;
    size_t offset = 0;
    while (offset < input.size()) {
        if (std::isspace(static_cast<unsigned char>(input[offset]))) {
            ++offset;
            continue;
        }
        std::cmatch match;
        if (!std::regex_search(
                input.c_str() + offset, match, token, std::regex_constants::match_continuous)) {
            return std::nullopt;
        }
        std::string word = match.str();
        if (word == "x") {
            word = "value";
            contains_x = true;
        } else if (word == "log" || word == "exp" || word == "sqrt" || word == "sin" ||
                   word == "cos" || word == "lgamma") {
            word = "math." + word;
        } else if (word == "pi") {
            word = "math.pi";
        } else if (word == "abs") {
            // Python's built-in abs() has the same scalar meaning.
        } else if (std::isalpha(static_cast<unsigned char>(word[0])) || word[0] == '_') {
            // Substitute only scalars assigned earlier in the active script.
            // Parenthesize substitutions so negative values preserve precedence.
            const auto found = variables.find(word);
            if (found == variables.end())
                return std::nullopt;
            std::ostringstream value;
            value << std::setprecision(17) << found->second;
            word = "(" + value.str() + ")";
        } else if (std::isdigit(static_cast<unsigned char>(word[0])) || word[0] == '.' ||
                   word[0] == ',') {
            std::replace(word.begin(), word.end(), ',', '.');
        }
        if (word == "(")
            ++depth;
        if (word == ")" && --depth < 0)
            return std::nullopt;
        result += word;
        offset += match.length();
    }
    if (depth != 0 || !contains_x) {
        return std::nullopt;
    }
    return result;
}
std::string trim(std::string s) {
    auto a = s.find_first_not_of(" \t\r\n");
    if (a == s.npos) {
        return {};
    }
    auto b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}
bool starts(const std::string& s, const std::string& p) {
    return s.rfind(p, 0) == 0;
}
// Gnuplot permits a quoted text argument immediately after its command name,
// for example `set title'Example'`, without intervening whitespace.
bool text_command(const std::string& s, const std::string& command) {
    if (!starts(s, command)) {
        return false;
    }
    if (s.size() == command.size()) {
        return true;
    }
    const unsigned char next = static_cast<unsigned char>(s[command.size()]);
    return std::isspace(next) || next == '\'' || next == '"';
}
std::string unquote(std::string s) {
    s = trim(s);
    if (s.size() >= 2 && (s.front() == '\'' || s.front() == '\"') && s.back() == s.front())
        return s.substr(1, s.size() - 2);
    return s;
}
std::string quoted(const std::string& s) {
    auto p = s.find_first_of("\"'");
    if (p == s.npos) {
        return trim(s);
    }
    auto q = s.find(s[p], p + 1);
    return q == s.npos ? trim(s.substr(p + 1)) : s.substr(p + 1, q - p - 1);
}
// In decimal-comma input, a comma between digits belongs to a numeric
// literal, not to the comma-separated list of plot specifications. Keep
// quoted titles intact and preserve normal splitting for dot-decimal files.
std::vector<std::string> split_specs(const std::string& s, bool decimal_comma = false) {
    std::vector<std::string> out;
    size_t begin = 0;
    char quote = 0;
    int depth = 0;
    for (size_t i = 0; i < s.size(); ++i) {
        const char c = s[i];
        if (quote) {
            if (c == quote && (i == 0 || s[i - 1] != '\\'))
                quote = 0;
        } else if (c == '\'' || c == '"') {
            quote = c;
        } else if (c == '(') {
            ++depth;
        } else if (c == ')') {
            --depth;
        } else if (c == ',' && depth == 0) {
            const bool numeric_comma = decimal_comma && i > 0 && i + 1 < s.size() &&
                                       std::isdigit(static_cast<unsigned char>(s[i - 1])) &&
                                       std::isdigit(static_cast<unsigned char>(s[i + 1]));
            if (!numeric_comma) {
                out.push_back(trim(s.substr(begin, i - begin)));
                begin = i + 1;
            }
        }
    }
    out.push_back(trim(s.substr(begin)));
    return out;
}
// Parse numeric tokens independently of the process-wide C/Python locale.
// A decimal comma is accepted only within a single numeric token.
std::optional<double> number(std::string s) {
    s = trim(s);
    if (s.empty() || s == "*" || s == "NaN" || s == "nan" || s == "?")
        return {};
    if (std::count(s.begin(), s.end(), ',') == 1 && s.find('.') == s.npos)
        std::replace(s.begin(), s.end(), ',', '.');
    try {
        size_t n = 0;
        const double v = std::stod(s, &n);
        if (n == s.size() && std::isfinite(v)) {
            return v;
        }
    } catch (...) {
    }
    return {};
}
void set_range(Axis& a, const std::string& s) {
    auto l = s.find('['), c = s.find(':', l), r = s.find(']', c);
    if (l == s.npos || c == s.npos || r == s.npos) {
        throw std::runtime_error("invalid range");
    }
    a.min = number(s.substr(l + 1, c - l - 1));
    a.max = number(s.substr(c + 1, r - c - 1));
}
// Numeric tic positions in libgretl spectra may contain pi, pi/4 or
// 3*pi/4. Recognize only these constant forms, without evaluating arbitrary
// gnuplot expressions or executing external code.
std::optional<double> tic_position(const std::string& text) {
    if (const auto numeric = number(text))
        return numeric;
    static const std::regex pi_tic(R"(^\s*(?:([0-9]+)\s*\*\s*)?pi(?:\s*/\s*([0-9]+))?\s*$)");
    std::smatch match;
    if (!std::regex_match(text, match, pi_tic))
        return {};
    const double numerator = match[1].matched ? std::stod(match[1].str()) : 1.0;
    const double denominator = match[2].matched ? std::stod(match[2].str()) : 1.0;
    if (denominator == 0)
        return {};
    return numerator * std::acos(-1.0) / denominator;
}
// Read libgretl's affine trigonometric x(t)/y(t) definitions. These
// equations describe the ellipse and its marginal confidence boundaries.
// Arbitrary gnuplot functions are not executed or silently approximated.
// Numbered function names and parenthesized signed terms are also used by
// contributed packages such as CUB for multiple confidence ellipses.
std::array<double, 3> ellipse_function(const std::string& text, const std::string& name) {
    static const std::string numeric =
        R"([+-]?(?:[0-9]+(?:\.[0-9]*)?|\.[0-9]+)(?:[eE][+-]?[0-9]+)?)";
    const std::regex definition("^" + name + R"(\(t\)\s*=\s*(.*)$)");
    std::smatch match;
    if (!std::regex_match(text, match, definition))
        throw std::runtime_error("unsupported parametric definition: " + text);
    std::string expression = match[1].str();
    // Gnuplot allows a signed trigonometric term in redundant parentheses,
    // e.g. A*cos(t) + (-B*sin(t)) + C. Remove only this validated form.
    expression.erase(std::remove_if(expression.begin(),
                                    expression.end(),
                                    [](unsigned char c) { return std::isspace(c); }),
                     expression.end());
    const std::regex parenthesized("\\((" + numeric + R"(\*(?:cos|sin)\(t\))\))");
    expression = std::regex_replace(expression, parenthesized, "$1");
    // Capture the two signed trigonometric coefficients and the intercept;
    // a plus/minus preceding a coefficient is applied separately.
    const std::regex affine("^(" + numeric + R"()\*cos\(t\)([+-])()" + numeric +
                            R"()\*sin\(t\)([+-])()" + numeric + ")$");
    if (!std::regex_match(expression, match, affine))
        throw std::runtime_error("unsupported parametric definition: " + text);
    const auto cos_value = number(match[1].str());
    const auto sin_value = number(match[3].str());
    const auto constant = number(match[5].str());
    if (!cos_value || !sin_value || !constant)
        throw std::runtime_error("invalid coefficient in parametric definition");
    return {*cos_value,
            (match[2] == "-" ? -1.0 : 1.0) * *sin_value,
            (match[4] == "-" ? -1.0 : 1.0) * *constant};
}
// Sample a restricted pair of parametric functions into the ordinary Plot IR.
// Parametric support is scoped to the syntax emitted by libgretl's confidence
// ellipse plot; unsupported expressions remain explicit translation errors.
std::vector<Series> ellipse_series(const std::string& command,
                                   const std::map<std::string, std::array<double, 3>>& functions,
                                   const std::pair<double, double>& trange,
                                   const std::vector<std::string>& colors) {
    const auto specs = split_specs(command);
    if (specs.size() % 2 != 0 || specs.empty())
        throw std::runtime_error("parametric plot requires x(t), y(t) pairs");
    std::vector<Series> series;
    static const std::regex xname(R"(^x[0-9]*\(t\)$)");
    static const std::regex yname(R"(^y[0-9]*\(t\)$)");
    for (size_t i = 0; i < specs.size(); i += 2) {
        const auto& xspec = specs[i];
        const auto& yspec = specs[i + 1];
        std::istringstream xs(xspec), ys(yspec);
        std::string xsource, ysource;
        xs >> xsource;
        ys >> ysource;
        const auto xconst = number(xsource);
        const auto yconst = number(ysource);
        if (!std::regex_match(xsource, xname) && !xconst)
            throw std::runtime_error("unsupported parametric x source: " + xspec);
        if (!std::regex_match(ysource, yname) && !yconst)
            throw std::runtime_error("unsupported parametric y source: " + yspec);
        // libgretl uses 'notitle' and optional 'w lines lt N'. Avoid
        // silently accepting other plot types as line graphs.
        static const std::regex parameters(
            R"(^\S+(?:\s+notitle)?(?:\s+(?:w|with)\s+(?:l|lines)(?:\s+lt\s+([1-9][0-9]*))?)?\s*$)");
        std::smatch match;
        if (trim(xspec) != xsource)
            throw std::runtime_error("invalid parametric x expression");
        if (!std::regex_match(yspec, match, parameters))
            throw std::runtime_error("unsupported parametric series style: " + yspec);
        // Resolve each numbered definition independently, so different curves
        // in one plot can have distinct centers and covariance matrices.
        const auto resolve = [&](const std::string& source,
                                 char axis) -> const std::array<double, 3>* {
            if (number(source))
                return nullptr;
            const auto name = source.substr(0, source.find('('));
            const auto found = functions.find(name);
            if (found == functions.end())
                throw std::runtime_error("undefined parametric " + std::string(1, axis) +
                                         " function: " + source);
            return &found->second;
        };
        const auto* xdef = resolve(xsource, 'x');
        const auto* ydef = resolve(ysource, 'y');
        Series ser;
        ser.title = "";
        ser.style = Style::Lines;
        if (match[1].matched) {
            ser.line_type = std::stoi(match[1].str());
            ser.explicit_color = true;
            if (ser.line_type > 0 && static_cast<size_t>(ser.line_type) <= colors.size())
                ser.color = colors[ser.line_type - 1];
            if (ser.color.empty())
                ser.color = "C" + std::to_string((ser.line_type - 1) % 10);
        }
        // Preserve the default gnuplot parametric range [-5:5] unless
        // the source supplies an explicit trange.
        for (int k = 0; k <= 400; ++k) {
            const double t = trange.first + (trange.second - trange.first) * k / 400;
            const double x = xconst
                               ? *xconst
                               : ((*xdef)[0] * std::cos(t) + (*xdef)[1] * std::sin(t) + (*xdef)[2]);
            const double y = yconst
                               ? *yconst
                               : ((*ydef)[0] * std::cos(t) + (*ydef)[1] * std::sin(t) + (*ydef)[2]);
            ser.points.push_back({x, y});
        }
        series.push_back(std::move(ser));
    }
    return series;
}
// Read a rectangular matrix, retaining row order and rejecting malformed cells.
ImageMatrix parse_matrix(const std::vector<std::string>& rows) {
    ImageMatrix matrix;
    size_t width = 0;
    for (const auto& raw : rows) {
        const auto row = trim(raw);
        if (row.empty() || row[0] == '#')
            continue;
        std::istringstream stream(row);
        std::vector<double> values;
        std::string cell;
        while (stream >> cell) {
            // NaN marks an absent matrix cell (e.g. the unused half of a
            // triangular correlation heatmap). Preserve it as missing data.
            if (cell == "NaN" || cell == "nan" || cell == "NAN") {
                values.push_back(std::numeric_limits<double>::quiet_NaN());
                continue;
            }
            auto value = number(cell);
            if (!value) {
                throw std::runtime_error("invalid numeric matrix value: " + cell);
            }
            values.push_back(*value);
        }
        if (values.empty()) {
            continue;
        }
        if (width == 0) {
            width = values.size();
        }
        if (values.size() != width)
            throw std::runtime_error("matrix rows have inconsistent column counts");
        matrix.rows.push_back(std::move(values));
    }
    if (matrix.rows.empty()) {
        throw std::runtime_error("empty matrix data source");
    }
    return matrix;
}
// Read a named data block as explicit XYZ points selected by 1-based columns.
// Blank rows delimit Gnuplot scans; dgrid3d treats all scans as scattered data.
std::vector<std::array<double, 3>> parse_xyz_points(const std::vector<std::string>& rows,
                                                    const std::array<int, 3>& columns) {
    std::vector<std::array<double, 3>> points;
    const int max_column = *std::max_element(columns.begin(), columns.end());
    for (const auto& raw : rows) {
        const auto line = trim(raw);
        if (line.empty() || line[0] == '#')
            continue;
        std::istringstream input(line);
        std::vector<std::string> cells;
        std::string cell;
        while (input >> cell) {
            if (!cell.empty() && cell[0] == '#')
                break;
            cells.push_back(cell);
        }
        if (static_cast<int>(cells.size()) < max_column)
            throw std::runtime_error("3D data row has too few columns: " + line);
        std::array<double, 3> point{};
        bool missing = false;
        for (size_t k = 0; k < columns.size(); ++k) {
            const auto value = number(cells[columns[k] - 1]);
            if (!value) {
                // Gnuplot omits points with undefined numeric coordinates.
                if (cells[columns[k] - 1] == "NaN" || cells[columns[k] - 1] == "nan" ||
                    cells[columns[k] - 1] == "NAN" || cells[columns[k] - 1] == "?" ||
                    cells[columns[k] - 1] == "*") {
                    missing = true;
                    break;
                }
                throw std::runtime_error("invalid 3D data value: " + cells[columns[k] - 1]);
            }
            point[k] = *value;
        }
        if (!missing)
            points.push_back(point);
    }
    if (points.size() < 3)
        throw std::runtime_error("3D XYZ surface requires at least three points");
    return points;
}
// Parse matrix labels with gnuplot pseudo-columns, sprintf formatting and
// a conditional RGB text color. Unsupported expressions remain errors.
// Some libgretl scripts also place plain numeric annotations with
// enhanced-text sizing (e.g. {/=12 \%.1f}).
MatrixLabels parse_matrix_labels(const std::string& spec, const std::vector<std::string>& rows) {
    // The four using expressions correspond to x, y, label and text color.
    // Whitespace is accepted around operators, without matching a plot name.
    // Also accept libgretl's NaN guard: $3!=$3 ? "" : sprintf(...).
    static const std::regex colored_expression(
        R"GP(^\$[A-Za-z_][A-Za-z0-9_]*\s+matrix\s+using\s+1\s*:\s*2\s*:\s*\(\s*(?:\$3\s*!=\s*\$3\s*\?\s*(?:""|'')\s*:\s*)?sprintf\s*\(\s*([\"'])(%[^\"']+)\1\s*,\s*\$3\s*\)\s*\)\s*:\s*\(\s*\$3\s*(<|<=|>|>=)\s*(-?[0-9]+(?:\.[0-9]+)?)\s*\?\s*(0x[0-9a-fA-F]{6})\s*:\s*(0x[0-9a-fA-F]{6})\s*\)\s+with\s+labels\s+tc\s+rgb\s+var\s*$)GP");
    // Simpler heatmap form: no conditional color, optional enhanced-text sizing.
    static const std::regex simple_expression(
        R"GP(^\$[A-Za-z_][A-Za-z0-9_]*\s+matrix\s+using\s+1\s*:\s*2\s*:\s*\(\s*sprintf\s*\(\s*([\"'])(.*?)\1\s*,\s*\$3\s*\)\s*\)\s+with\s+labels\s*$)GP");
    static const std::regex enhanced_size(R"GP(^\{/=\s*([0-9]+)\s+(.+)\}$)GP");
    static const std::regex numeric_format(R"(^%[-+ #0]*[0-9]*(?:\.[0-9]+)?[eEfFgG]$)");

    std::smatch match;
    MatrixLabels labels;
    if (std::regex_match(spec, match, colored_expression)) {
        labels.format = match[2].str();
        labels.conditional_color = true;
        labels.comparison = match[3].str();
        labels.threshold = std::stod(match[4].str());
        labels.below_color = match[5].str().substr(2);
        labels.otherwise_color = match[6].str().substr(2);
        // Normalize the condition to a less-than comparison by swapping colors
        // where possible. The operator is stored separately below instead.
    } else if (std::regex_match(spec, match, simple_expression)) {
        labels.format = match[2].str();
        std::smatch font_match;
        if (std::regex_match(labels.format, font_match, enhanced_size)) {
            labels.font_size = std::stoi(font_match[1].str());
            labels.format = font_match[2].str();
        }
        // The escape before % belongs to gnuplot's enhanced-text syntax,
        // not to the numeric format used by Python's %-formatter.
        while (!labels.format.empty() && labels.format.front() == '\\')
            labels.format.erase(0, 1);
    } else {
        throw std::runtime_error("unsupported matrix plot expression: " + spec);
    }

    // Python's %-formatting handles standard numeric printf formats.
    if (!std::regex_match(labels.format, numeric_format))
        throw std::runtime_error("unsupported numeric label format: " + labels.format);
    labels.matrix = parse_matrix(rows);
    return labels;
}
// Parse gnuplot's piecewise RGB palette. Stops can use named colors or
// normalized numeric R G B triples; mixed forms are accepted.
std::vector<PaletteStop> parse_palette(const std::string& definition) {
    const auto left = definition.find('(');
    const auto right = definition.find_last_of(')');
    if (left == std::string::npos || right == std::string::npos || right <= left)
        throw std::runtime_error("invalid palette definition");
    if (!trim(definition.substr(right + 1)).empty())
        throw std::runtime_error("unsupported palette suffix");
    std::vector<PaletteStop> stops;
    for (const auto& item : split_specs(definition.substr(left + 1, right - left - 1))) {
        std::smatch match;
        static const std::regex named_re(R"(^\s*([^\s]+)\s+(['\"])(.*?)\2\s*$)");
        static const std::regex rgb_re(R"(^\s*([^\s]+)\s+([^\s]+)\s+([^\s]+)\s+([^\s]+)\s*$)");
        PaletteStop stop;
        if (std::regex_match(item, match, named_re)) {
            const auto value = number(match[1].str());
            if (!value)
                throw std::runtime_error("invalid palette breakpoint: " + item);
            stop.value = *value;
            stop.color = match[3].str();
        } else if (std::regex_match(item, match, rgb_re)) {
            const auto value = number(match[1].str());
            const auto red = number(match[2].str());
            const auto green = number(match[3].str());
            const auto blue = number(match[4].str());
            if (!value || !red || !green || !blue || *red < 0 || *red > 1 || *green < 0 ||
                *green > 1 || *blue < 0 || *blue > 1)
                throw std::runtime_error("invalid RGB palette stop: " + item);
            stop.value = *value;
            stop.rgb = std::array<double, 3>{*red, *green, *blue};
        } else {
            throw std::runtime_error("invalid palette stop: " + item);
        }
        if (!stops.empty() && stop.value <= stops.back().value)
            throw std::runtime_error("palette breakpoints must be strictly increasing");
        stops.push_back(std::move(stop));
    }
    if (stops.size() < 2)
        throw std::runtime_error("palette requires at least two color stops");
    return stops;
}
// Split several gnuplot commands written on one line, without splitting
// quoted semicolons or separators embedded inside parentheses.
std::vector<std::string> split_commands(const std::string& line) {
    std::vector<std::string> commands;
    char quote = 0;
    int depth = 0;
    size_t begin = 0;
    for (size_t i = 0; i < line.size(); ++i) {
        const char c = line[i];
        if (quote) {
            if (c == quote && (i == 0 || line[i - 1] != '\\'))
                quote = 0;
        } else if (c == '\'' || c == '"') {
            quote = c;
        } else if (c == '(') {
            ++depth;
        } else if (c == ')' && depth > 0) {
            --depth;
        } else if (c == ';' && depth == 0) {
            commands.push_back(trim(line.substr(begin, i - begin)));
            begin = i + 1;
        }
    }
    commands.push_back(trim(line.substr(begin)));
    return commands;
}
// Parse named tic positions without conflating commas inside quoted labels.
void parse_tics(Tics& tics, const std::string& spec) {
    tics.enabled = true;
    const auto items_begin = spec.find('(');
    if (items_begin != std::string::npos && trim(spec.substr(0, items_begin)).empty()) {
        const auto items_end = spec.find(')', items_begin);
        if (items_end == std::string::npos) {
            throw std::runtime_error("unterminated tic list");
        }
        tics.explicit_tics.clear();
        for (const auto& item :
             split_specs(spec.substr(items_begin + 1, items_end - items_begin - 1))) {
            std::smatch match;
            static const std::regex tic_re(R"(^\s*(['"])(.*?)\1\s+([^\s]+)\s*$)");
            if (!std::regex_match(item, match, tic_re))
                throw std::runtime_error("invalid tic specification: " + item);
            const auto pos = tic_position(match[3].str());
            if (!pos) {
                throw std::runtime_error("invalid tic position: " + item);
            }
            tics.explicit_tics.push_back({match[2].str(), *pos});
        }
        parse_tics(tics, trim(spec.substr(items_end + 1)));
        return;
    }
    std::smatch match;
    if (std::regex_search(
            spec, match, std::regex(R"((?:^|\s)rotate\s+by\s+(-?[0-9]+(?:\.[0-9]+)?))")))
        tics.rotation = std::stod(match[1].str());
    if (std::regex_search(spec, match, std::regex(R"((?:^|\s)format\s+(['"])(.*?)\1)")))
        tics.format = match[2].str();
    if (std::regex_search(spec, match, std::regex(R"((?:^|\s)scale\s+([^\s]+))"))) {
        const auto parts = split_specs(match[1].str());
        if (parts.size() > 2 || parts.empty()) {
            throw std::runtime_error("invalid tic scale");
        }
        tics.major_scale = number(parts[0]);
        if (!tics.major_scale) {
            throw std::runtime_error("invalid major tic scale");
        }
        if (parts.size() == 2) {
            tics.minor_scale = number(parts[1]);
            if (!tics.minor_scale) {
                throw std::runtime_error("invalid minor tic scale");
            }
        }
    }
    // A font modifier such as font ',7' specifies tick-label size.
    static const std::regex font_re(
        R"GP((?:^|\s)font\s+(['"])([^'",]*),([0-9]+(?:\.[0-9]+)?)\1(?=\s|$))GP");
    if (std::regex_search(spec, match, font_re)) {
        tics.font_size = std::stod(match[3].str());
        if (*tics.font_size <= 0)
            throw std::runtime_error("invalid tic font size");
    }
    // The default gnuplot offset coordinates are character cells. Preserve
    // both components separately: they move the labels, not the tick marks.
    // Non-character coordinate systems require a different conversion.
    static const std::regex tic_offset_re(
        R"((?:^|\s)offset\s+(?:character\s+)?([+-]?(?:[0-9]+(?:\.[0-9]*)?|\.[0-9]+)(?:[eE][+-]?[0-9]+)?)\s*,\s*([+-]?(?:[0-9]+(?:\.[0-9]*)?|\.[0-9]+)(?:[eE][+-]?[0-9]+)?)(?=\s|$))");
    if (std::regex_search(spec, match, tic_offset_re)) {
        const double dx = std::stod(match[1].str());
        const double dy = std::stod(match[2].str());
        if (!std::isfinite(dx) || !std::isfinite(dy))
            throw std::runtime_error("invalid tic offset");
        tics.label_offset = std::array<double, 2>{dx, dy};
    }
    if (std::regex_search(spec, std::regex(R"((?:^|\s)nooffset(?=\s|$))")))
        tics.label_offset.reset();
    // `in` and `out` are mutually exclusive; later directives win.
    if (std::regex_search(spec, std::regex(R"((?:^|\s)in(?:\s|$))"))) {
        tics.outward = false;
        tics.direction_explicit = true;
    }
    if (std::regex_search(spec, std::regex(R"((?:^|\s)out(?:\s|$))"))) {
        tics.outward = true;
        tics.direction_explicit = true;
    }
    // A numeric tic specification is incr or start,incr[,end].
    // Keep previously configured properties (such as nomirror) intact.
    static const std::regex numeric_tics_re(
        R"(^\s*([+-]?(?:[0-9]+(?:\.[0-9]*)?|\.[0-9]+)(?:[eE][+-]?[0-9]+)?)(?:\s*,\s*([+-]?(?:[0-9]+(?:\.[0-9]*)?|\.[0-9]+)(?:[eE][+-]?[0-9]+)?)(?:\s*,\s*([+-]?(?:[0-9]+(?:\.[0-9]*)?|\.[0-9]+)(?:[eE][+-]?[0-9]+)?))?)?(?=\s|$))");
    if (std::regex_search(spec, match, numeric_tics_re)) {
        const bool has_start = match[2].matched;
        const double step = std::stod(has_start ? match[2].str() : match[1].str());
        if (step <= 0 || !std::isfinite(step))
            throw std::runtime_error("tic increment must be positive and finite");
        tics.interval = step;
        tics.start = has_start ? std::optional<double>(std::stod(match[1].str())) : std::nullopt;
        tics.end =
            match[3].matched ? std::optional<double>(std::stod(match[3].str())) : std::nullopt;
    }
    // Reject unimplemented modifiers rather than silently dropping semantics.
    auto rest = std::regex_replace(
        spec, std::regex(R"((?:^|\s)rotate\s+by\s+-?[0-9]+(?:\.[0-9]+)?(?:\s+right)?)"), " ");
    rest = std::regex_replace(rest, std::regex(R"((?:^|\s)format\s+(['"])(.*?)\1)"), " ");
    rest = std::regex_replace(rest, std::regex(R"((?:^|\s)scale\s+[^\s]+)"), " ");
    rest = std::regex_replace(rest, font_re, " ");
    rest = std::regex_replace(rest, tic_offset_re, " ");
    rest = std::regex_replace(rest, std::regex(R"((?:^|\s)nooffset(?=\s|$))"), " ");
    rest = std::regex_replace(rest, std::regex(R"((?:^|\s)in(?:\s|$))"), " ");
    rest = std::regex_replace(rest, std::regex(R"((?:^|\s)out(?:\s|$))"), " ");
    // Mirroring is a per-axis property, not an unsupported tic modifier.
    if (std::regex_search(spec, std::regex(R"((?:^|\s)nomirror(?:\s|$))")))
        tics.mirror = false;
    if (std::regex_search(spec, std::regex(R"((?:^|\s)mirror(?:\s|$))")))
        tics.mirror = true;
    rest = std::regex_replace(rest, std::regex(R"((?:^|\s)nomirror(?:\s|$))"), " ");
    rest = std::regex_replace(rest, std::regex(R"((?:^|\s)mirror(?:\s|$))"), " ");
    rest = std::regex_replace(rest, numeric_tics_re, " ");
    if (!trim(rest).empty()) {
        throw std::runtime_error("unsupported tic modifier: " + trim(rest));
    }
}
// Named gnuplot line styles are distinct from linetype palette entries.
struct NamedLineStyle {
    std::string color;
    int line_type{1};
    double width{1.5};
};
std::string keyword_value(const std::string& s, const std::string& key) {
    std::regex re("(?:^|\\s)" + key + "\\s+((?:\\\"[^\\\"]*\\\")|(?:'[^']*')|(?:[^\\s,]+))");
    std::smatch m;
    return std::regex_search(s, m, re) ? unquote(m[1].str()) : "";
}
// Return the start of the column specification for 'using' or its 'u' alias.
std::optional<size_t> using_offset(const std::string& spec) {
    const auto pos = spec.find(" using ");
    if (pos != spec.npos)
        return pos + 7;
    const auto short_pos = spec.find(" u ");
    if (short_pos != spec.npos)
        return short_pos + 3;
    return std::nullopt;
}
int using_col(const std::string& spec, int default_col, bool first) {
    const auto pos = using_offset(spec);
    if (!pos) {
        return default_col;
    }
    auto t = trim(spec.substr(*pos));
    std::smatch m;
    std::regex re(R"(^(\d+)(?::(\d+))?)");
    if (!std::regex_search(t, m, re)) {
        throw std::runtime_error("unsupported 'using' expression");
    }
    if (first) {
        return m[2].matched ? std::stoi(m[1].str()) : 0;
    }
    return m[2].matched ? std::stoi(m[2].str()) : std::stoi(m[1].str());
}
// Evaluate a restricted gnuplot column expression: $N, constants, +/- and
// parentheses. Unsupported syntax is rejected rather than evaluated as
// arbitrary code.
class ColumnExpression {
  public:
    ColumnExpression(const std::string& expression, const std::vector<std::string>& cells)
        : expression_(expression), cells_(cells) {
    }
    std::optional<double> evaluate() {
        auto value = sum();
        spaces();
        if (pos_ != expression_.size())
            throw std::runtime_error("unsupported using expression: " + expression_);
        return value;
    }

  private:
    void spaces() {
        while (pos_ < expression_.size() &&
               std::isspace(static_cast<unsigned char>(expression_[pos_])))
            ++pos_;
    }
    std::optional<double> sum() {
        auto lhs = product();
        while (true) {
            spaces();
            if (pos_ == expression_.size() ||
                (expression_[pos_] != '+' && expression_[pos_] != '-'))
                return lhs;
            char op = expression_[pos_++];
            auto rhs = product();
            if (!lhs || !rhs)
                lhs = std::nullopt;
            else
                lhs = op == '+' ? *lhs + *rhs : *lhs - *rhs;
        }
    }
    std::optional<double> product() {
        auto lhs = atom();
        while (true) {
            spaces();
            if (pos_ == expression_.size() ||
                (expression_[pos_] != '*' && expression_[pos_] != '/'))
                return lhs;
            char op = expression_[pos_++];
            auto rhs = atom();
            if (!lhs || !rhs || (op == '/' && *rhs == 0))
                lhs = std::nullopt;
            else
                lhs = op == '*' ? *lhs * *rhs : *lhs / *rhs;
        }
    }
    std::optional<double> atom() {
        spaces();
        if (pos_ == expression_.size())
            throw std::runtime_error("incomplete using expression: " + expression_);
        if (expression_[pos_] == '+' || expression_[pos_] == '-') {
            char sign = expression_[pos_++];
            auto value = atom();
            return value && sign == '-' ? std::optional<double>(-*value) : value;
        }
        if (expression_[pos_] == '(') {
            ++pos_;
            auto value = sum();
            spaces();
            if (pos_ == expression_.size() || expression_[pos_++] != ')')
                throw std::runtime_error("unbalanced using expression: " + expression_);
            return value;
        }
        if (expression_[pos_] == '$') {
            ++pos_;
            size_t start = pos_;
            while (pos_ < expression_.size() &&
                   std::isdigit(static_cast<unsigned char>(expression_[pos_])))
                ++pos_;
            if (start == pos_)
                throw std::runtime_error("invalid column reference: " + expression_);
            size_t col = std::stoul(expression_.substr(start, pos_ - start));
            return col > 0 && col <= cells_.size() ? number(cells_[col - 1]) : std::nullopt;
        }
        size_t start = pos_;
        while (pos_ < expression_.size() &&
               (std::isdigit(static_cast<unsigned char>(expression_[pos_])) ||
                expression_[pos_] == '.'))
            ++pos_;
        if (start == pos_)
            throw std::runtime_error("unsupported using expression: " + expression_);
        return number(expression_.substr(start, pos_ - start));
    }
    const std::string& expression_;
    const std::vector<std::string>& cells_;
    size_t pos_{0};
};
// Split using fields without breaking expressions inside parentheses.
std::vector<std::string> using_fields(const std::string& spec, size_t expected = 3) {
    const auto start = using_offset(spec);
    if (!start)
        throw std::runtime_error("missing using specification");
    const std::string tail = spec.substr(*start);
    std::vector<std::string> fields;
    int depth = 0;
    size_t begin = 0;
    for (size_t i = 0; i < tail.size(); ++i) {
        if (tail[i] == '(')
            ++depth;
        else if (tail[i] == ')')
            --depth;
        else if (tail[i] == ':' && depth == 0) {
            fields.push_back(trim(tail.substr(begin, i - begin)));
            begin = i + 1;
        } else if (std::isspace(static_cast<unsigned char>(tail[i])) && depth == 0 &&
                   fields.size() == expected - 1) {
            break;
        }
    }
    // The last field ends at the first top-level whitespace (plot modifiers).
    depth = 0;
    size_t end = begin;
    for (; end < tail.size(); ++end) {
        if (tail[end] == '(')
            ++depth;
        else if (tail[end] == ')')
            --depth;
        else if (depth == 0 && std::isspace(static_cast<unsigned char>(tail[end])))
            break;
    }
    fields.push_back(trim(tail.substr(begin, end - begin)));
    if (fields.size() != expected || depth != 0)
        throw std::runtime_error("invalid number of using fields");
    // Bare integers in gnuplot using fields refer to columns, not constants.
    for (auto& field : fields) {
        if (std::regex_match(field, std::regex(R"([1-9][0-9]*)")))
            field.insert(field.begin(), '$');
    }
    return fields;
}
// Evaluate a using field as a column reference or a restricted expression.
std::optional<double> evaluate_using_field(const std::string& field,
                                           const std::vector<std::string>& cells) {
    const std::string expr = trim(field);

    // Bare integers denote 1-based column indices, not constants.
    if (!expr.empty() &&
        std::all_of(expr.begin(), expr.end(), [](unsigned char c) { return std::isdigit(c); })) {
        const size_t col = std::stoul(expr);
        return col > 0 && col <= cells.size() ? number(cells[col - 1]) : std::nullopt;
    }

    // The existing expression evaluator handles $N, arithmetic and parentheses.
    return ColumnExpression(expr, cells).evaluate();
}
} // namespace
// Evaluate scalar expressions used by assignments and conditional branches.
// Unknown identifiers and unsupported syntax are errors, never implicit zeroes.
double scalar_value(const std::string& expression, const std::map<std::string, double>& variables) {
    const auto token = trim(expression);
    if (const auto value = number(token)) {
        return *value;
    }
    static const std::regex identifier(R"(^[A-Za-z_][A-Za-z0-9_]*$)");
    if (std::regex_match(token, identifier)) {
        const auto found = variables.find(token);
        if (found == variables.end())
            throw std::runtime_error("undefined variable: " + token);
        return found->second;
    }
    throw std::runtime_error("unsupported scalar expression: " + token);
}
// Support scalar truth values and comparisons without evaluating arbitrary
// code.
bool condition_value(const std::string& expression,
                     const std::map<std::string, double>& variables) {
    const auto condition = trim(expression);
    std::smatch match;
    static const std::regex comparison(R"(^\s*(.*?)\s*(==|!=|<=|>=|<|>)\s*(.*?)\s*$)");
    if (std::regex_match(condition, match, comparison)) {
        const double lhs = scalar_value(match[1].str(), variables);
        const double rhs = scalar_value(match[3].str(), variables);
        const auto op = match[2].str();
        if (op == "==")
            return lhs == rhs;
        if (op == "!=")
            return lhs != rhs;
        if (op == "<=")
            return lhs <= rhs;
        if (op == ">=")
            return lhs >= rhs;
        if (op == "<")
            return lhs < rhs;
        return lhs > rhs;
    }
    return scalar_value(condition, variables) != 0.0;
}
// Interpret control-flow syntax before the plot parser. Retain line positions
// by blanking control statements and inactive branches instead of deleting
// rows.
void resolve_conditionals(std::vector<std::string>& lines, std::vector<Diagnostic>& errors) {
    const auto original = lines;
    std::map<std::string, double> variables;
    static const std::regex assignment(R"(^([A-Za-z_][A-Za-z0-9_]*)\s*=\s*(.+)$)");
    static const std::regex if_open(R"(^if\s*\((.*)\)\s*\{$)");
    size_t index = 0;
    // A block consumes its closing brace. The caller handles an optional else.
    std::function<void(bool, bool)> block = [&](bool active, bool nested) {
        while (index < original.size()) {
            const size_t line_number = index + 1;
            const auto statement = trim(original[index]);
            std::smatch match;
            // Data block contents are literal input, not executable statements.
            std::smatch data_block;
            static const std::regex data_open(
                R"(^\$[A-Za-z_][A-Za-z0-9_]*\s*<<\s*([A-Za-z_][A-Za-z0-9_]*)\s*$)");
            if (std::regex_match(statement, data_block, data_open)) {
                const std::string terminator = data_block[1].str();
                if (!active) {
                    lines[index].clear();
                }
                ++index;
                while (index < original.size() && trim(original[index]) != terminator) {
                    if (!active) {
                        lines[index].clear();
                    }
                    ++index;
                }
                if (index < original.size()) {
                    if (!active) {
                        lines[index].clear();
                    }
                    ++index;
                }
                continue;
            }
            if (statement == "}" || statement == "} else {") {
                if (!nested) {
                    errors.push_back({line_number, "unexpected conditional closing brace"});
                    lines[index++].clear();
                    continue;
                }
                return;
            }
            if (std::regex_match(statement, match, if_open)) {
                bool branch = false;
                if (active) {
                    try {
                        branch = condition_value(match[1].str(), variables);
                    } catch (const std::exception& e) {
                        errors.push_back({line_number, e.what()});
                    }
                }
                lines[index++].clear();
                block(active && branch, true);
                if (index >= original.size()) {
                    errors.push_back({line_number, "unterminated if block"});
                    return;
                }
                const bool has_else = trim(original[index]) == "} else {";
                lines[index++].clear();
                if (has_else) {
                    block(active && !branch, true);
                    if (index >= original.size() || trim(original[index]) != "}") {
                        errors.push_back({line_number, "unterminated else block"});
                        return;
                    }
                    lines[index++].clear();
                }
                continue;
            }
            if (std::regex_match(statement, match, assignment)) {
                if (active) {
                    try {
                        variables[match[1].str()] = scalar_value(match[2].str(), variables);
                    } catch (const std::exception& e) {
                        errors.push_back({line_number, e.what()});
                    }
                }
                // Keep active assignments for the plot parser: theoretical
                // distributions refer to sigma, mu, alpha and beta later on.
                if (!active)
                    lines[index].clear();
                ++index;
                continue;
            }
            if (!active) {
                lines[index].clear();
            }
            ++index;
        }
    };
    block(true, false);
}
ParseResult parse(const std::string& source) {
    ParseResult result;
    auto& d = result.document;
    d.plots.emplace_back();
    Plot* p = &d.plots.back();
    // Gnuplot defaults to points for data and lines for functions.
    Style default_style = Style::Points;
    bool multiplot_layout = false;
    bool parametric = false;
    // Keep numbered parametric definitions across successive plot commands.
    std::map<std::string, std::array<double, 3>> parametric_functions;
    std::pair<double, double> trange{-5.0, 5.0};
    std::string previous_file;
    std::map<std::string, std::vector<std::string>> external_data;
    // Keep scalar definitions local and in source order. The conditional pass
    // selects active lines; this pass resolves values at each plot command.
    std::map<std::string, double> plot_scalars;
    std::istringstream input(source);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        lines.push_back(line);
    }
    resolve_conditionals(lines, result.errors);
    // Expand command chains only outside named data blocks. Record source line
    // numbers so diagnostics still point to the original gnuplot script.
    std::vector<std::string> expanded;
    std::vector<size_t> source_lines;
    std::string block_terminator;
    static const std::regex data_declaration(
        R"(^\$[A-Za-z_][A-Za-z0-9_]*\s*<<\s*([A-Za-z_][A-Za-z0-9_]*)\s*$)");
    for (size_t source_line = 0; source_line < lines.size(); ++source_line) {
        const auto raw = trim(lines[source_line]);
        std::smatch declaration;
        if (!block_terminator.empty()) {
            expanded.push_back(lines[source_line]);
            source_lines.push_back(source_line + 1);
            if (raw == block_terminator)
                block_terminator.clear();
        } else if (std::regex_match(raw, declaration, data_declaration)) {
            block_terminator = declaration[1].str();
            expanded.push_back(lines[source_line]);
            source_lines.push_back(source_line + 1);
        } else if (raw.empty() || raw[0] == '#') {
            // Comments and empty lines are not command chains.
            expanded.push_back(lines[source_line]);
            source_lines.push_back(source_line + 1);
        } else {
            // Gnuplot's trailing backslash joins physical source lines into
            // one logical command. This is needed for multi-line RGB palettes;
            // never join the literal contents of a named data block.
            const size_t first_line = source_line + 1;
            std::string command_text = lines[source_line];
            while (!trim(command_text).empty() && trim(command_text).back() == '\\') {
                auto part = trim(command_text);
                part.pop_back();
                if (source_line + 1 >= lines.size()) {
                    result.errors.push_back({first_line, "unfinished line continuation"});
                    command_text = std::move(part);
                    break;
                }
                command_text = part + " " + trim(lines[++source_line]);
            }
            for (const auto& command : split_commands(command_text)) {
                expanded.push_back(command);
                source_lines.push_back(first_line);
            }
        }
    }
    lines = std::move(expanded);
    // Detect gretl's decimal-comma numeric output before splitting plot lists.
    // Axis ranges and inline data rows provide unambiguous evidence; quoted
    // labels and syntactic commas (e.g. `set size 1.0,1.0`) do not.
    bool decimal_comma = false;
    const std::regex range_comma(R"(^set (?:x|y|x2|y2)range\s*\[[^\]]*[0-9],[0-9])");
    const std::regex data_comma(R"(^[+-]?[0-9]+\s+[+-]?[0-9]+,[0-9]+(?:\s|$))");
    for (const auto& raw : lines) {
        const auto candidate = trim(raw);
        if (std::regex_search(candidate, range_comma) || std::regex_search(candidate, data_comma)) {
            decimal_comma = true;
            break;
        }
    }
    std::map<int, NamedLineStyle> named_line_styles;
    // Preserve the 2D contour view as parser state. A real 3D splot stays
    // unsupported until the separate surface backend is implemented.
    bool contour_base = false;
    bool contour_view_map = false;
    bool contour_surface = true;
    // `set pm3d at s` enables filled 3D surfaces, not flat contour maps.
    bool pm3d_surface = false;
    // The gridding directive applies to XYZ sources, not matrix sources.
    std::optional<std::array<int, 3>> dgrid3d;
    int contour_level_count = 0;
    int contour_first_linetype = 0;
    std::vector<double> contour_levels;
    for (size_t i = 0; i < lines.size(); ++i) {
        auto s = trim(lines[i]);
        if (s.empty() || s[0] == '#')
            continue;
        // A named data block is a declaration, not a plotting command.
        // Its terminator is taken literally and the data rows are retained intact.
        if (starts(s, "$")) {
            std::smatch block;
            if (std::regex_match(
                    s,
                    block,
                    std::regex(
                        R"(^(\$[A-Za-z_][A-Za-z0-9_]*)\s*<<\s*([A-Za-z_][A-Za-z0-9_]*)\s*$)"))) {
                const std::string name = block[1].str(), terminator = block[2].str();
                std::vector<std::string> rows;
                const size_t declaration_line = source_lines[i];
                bool closed = false;
                while (++i < lines.size()) {
                    if (trim(lines[i]) == terminator) {
                        closed = true;
                        break;
                    }
                    rows.push_back(lines[i]);
                }
                if (!closed) {
                    result.errors.push_back({declaration_line, "unterminated data block: " + name});
                } else
                    d.data_blocks[name] = std::move(rows);
                continue;
            }
        }
        try {
            // Preserve active scalar assignments in the plotting environment.
            // Never execute arbitrary gnuplot text as Python source.
            std::smatch scalar_assignment;
            static const std::regex scalar_pattern(R"(^([A-Za-z_][A-Za-z0-9_]*)\s*=\s*(.+)$)");
            if (std::regex_match(s, scalar_assignment, scalar_pattern)) {
                plot_scalars[scalar_assignment[1].str()] =
                    scalar_value(scalar_assignment[2].str(), plot_scalars);
                continue;
            }
            // Set the default size for subsequent point-bearing plot items.
            if (starts(s, "set pointsize ")) {
                const auto size = number(trim(s.substr(14)));
                if (!size || !std::isfinite(*size) || *size <= 0)
                    throw std::runtime_error("invalid pointsize: " + s);
                p->point_size = *size;
                continue;
            }
            // Line styles (ls N) have a separate namespace from linetypes.
            if (starts(s, "set style line ")) {
                static const std::regex named_style_re(
                    R"GP(^set style line\s+([1-9][0-9]*)\s+lc\s+rgb\s+(['"])(.*?)\2\s+lt\s+(-?[0-9]+)\s+lw\s+([0-9]+(?:\.[0-9]+)?)\s*$)GP");
                std::smatch m;
                if (!std::regex_match(s, m, named_style_re))
                    throw std::runtime_error("unsupported style line definition: " + s);
                NamedLineStyle style;
                style.color = m[3].str();
                style.line_type = std::stoi(m[4].str());
                style.width = std::stod(m[5].str());
                if (style.width <= 0)
                    throw std::runtime_error("invalid style line width: " + s);
                named_line_styles[std::stoi(m[1].str())] = std::move(style);
                continue;
            }
            // Linetype definitions are global and may precede the plot command.
            if (starts(s, "set linetype ")) {
                std::smatch m;
                if (std::regex_search(s, m, std::regex(R"(^set linetype\s+(\d+))"))) {
                    auto n = std::stoi(m[1]);
                    if (n > 0 && n <= 256) {
                        if (d.linetype_colors.size() < static_cast<size_t>(n)) {
                            d.linetype_colors.resize(n);
                        }
                        // Some packages use `lc "#rrggbb"` without the optional
                        // `rgb` keyword. Both forms define the same color.
                        auto color = keyword_value(s, "lc rgb");
                        if (color.empty())
                            color = keyword_value(s, "lc");
                        d.linetype_colors[n - 1] = color;
                        // Keep the per-level line width for 2D contour plots.
                        if (const auto width = number(keyword_value(s, "lw"))) {
                            if (*width <= 0 || !std::isfinite(*width))
                                throw std::runtime_error("invalid linetype width: " + s);
                            if (d.linetype_widths.size() < static_cast<size_t>(n))
                                d.linetype_widths.resize(n, 1.5);
                            d.linetype_widths[n - 1] = *width;
                        }
                    }
                }
                continue;
            }
            // Border visibility shares the bitmask used by `set border N`.
            if (s == "unset border") {
                p->border = 0;
                continue;
            }
            if (s == "set border") {
                p->border = 15;
                continue;
            }
            if (starts(s, "set border ")) {
                std::istringstream bs(s.substr(11));
                bs >> p->border;
                p->border_color = keyword_value(s, "lc rgb");
                continue;
            }
            if (s == "set tics nomirror") {
                p->mirror_tics = false;
                continue;
            }
            if (s == "set tics mirror") {
                p->mirror_tics = true;
                continue;
            }
            // Global tics directives are shared by the primary axes. In
            // particular, heatmap uses `nomirror in font ",12"`.
            if (starts(s, "set tics ")) {
                const auto spec = trim(s.substr(std::string("set tics ").size()));
                parse_tics(p->x.tics, spec);
                parse_tics(p->y.tics, spec);
                // Global tic settings also apply to the Z axis of a 3D plot.
                parse_tics(p->z.tics, spec);
                p->mirror_tics = p->x.tics.mirror && p->y.tics.mirror;
                continue;
            }
            // Graph offsets are fractions of the plot range; bare numeric
            // offsets are additive data units. Preserve the distinction.
            if (starts(s, "set offsets ")) {
                const auto parts = split_specs(trim(s.substr(std::string("set offsets ").size())));
                if (parts.size() != 4)
                    throw std::runtime_error("unsupported offsets expression: " + s);
                std::array<double, 4> offsets{};
                const bool graph_units = starts(parts[0], "graph ");
                for (size_t k = 0; k < offsets.size(); ++k) {
                    if (starts(parts[k], "graph ") != graph_units)
                        throw std::runtime_error("mixed offsets units are not supported: " + s);
                    const auto value = number(graph_units ? parts[k].substr(6) : parts[k]);
                    if (!value || !std::isfinite(*value))
                        throw std::runtime_error("invalid offsets: " + parts[k]);
                    if (graph_units && (*value < 0 || *value >= 1))
                        throw std::runtime_error("invalid graph offsets: " + parts[k]);
                    offsets[k] = *value;
                }
                if (graph_units && (offsets[0] + offsets[1] >= 1 || offsets[2] + offsets[3] >= 1))
                    throw std::runtime_error("graph offsets exceed available plot area");
                p->graph_offsets = graph_units ? std::optional(offsets) : std::nullopt;
                p->axis_offsets = graph_units ? std::nullopt : std::optional(offsets);
                continue;
            }
            if (s == "unset offsets" || s == "set nooffsets") {
                p->graph_offsets.reset();
                p->axis_offsets.reset();
                continue;
            }
            if (s == "set xzeroaxis") {
                p->xzeroaxis = true;
                continue;
            }
            if (s == "unset xzeroaxis" || s == "set noxzeroaxis") {
                p->xzeroaxis = false;
                continue;
            }
            // `yzeroaxis` is the vertical reference axis x=0 (not y=0).
            if (s == "set yzeroaxis") {
                p->yzeroaxis = true;
                continue;
            }
            if (s == "unset yzeroaxis" || s == "set noyzeroaxis") {
                p->yzeroaxis = false;
                continue;
            }
            // Parameterized x(t)/y(t) curves are used for confidence ellipses.
            if (s == "set parametric") {
                parametric = true;
                continue;
            }
            if (s == "unset parametric") {
                parametric = false;
                continue;
            }
            if (starts(s, "set trange ")) {
                // Gnuplot permits pi-based bounds in parametric ranges,
                // e.g. [0:2*pi] or [-pi:pi]. Reuse the restricted numeric
                // tic parser rather than evaluating general expressions.
                const auto left = s.find('[');
                const auto colon = s.find(':', left);
                const auto right = s.find(']', colon);
                if (left == s.npos || colon == s.npos || right == s.npos)
                    throw std::runtime_error("invalid parametric trange: " + s);
                const auto bound = [](std::string value) -> std::optional<double> {
                    value = trim(value);
                    if (auto result = tic_position(value))
                        return result;
                    // tic_position() handles unsigned multiples of pi;
                    // parametric bounds may also have a unary sign.
                    if (!value.empty() && (value.front() == '-' || value.front() == '+')) {
                        if (auto result = tic_position(value.substr(1)))
                            return value.front() == '-' ? -*result : *result;
                    }
                    return {};
                };
                const auto minimum = bound(s.substr(left + 1, colon - left - 1));
                const auto maximum = bound(s.substr(colon + 1, right - colon - 1));
                if (!minimum || !maximum || *maximum <= *minimum)
                    throw std::runtime_error("invalid parametric trange: " + s);
                trange = {*minimum, *maximum};
                continue;
            }
            // Restrict assignments to the xN(t)/yN(t) ellipse family.
            static const std::regex parametric_definition(R"(^([xy][0-9]*)\(t\)\s*=)");
            std::smatch function_match;
            if (std::regex_search(s, function_match, parametric_definition)) {
                const auto name = function_match[1].str();
                parametric_functions[name] = ellipse_function(s, name);
                continue;
            }
            // A polar plot's inline columns contain theta (radians) and r.
            if (s == "set polar") {
                p->polar = true;
                continue;
            }
            if (s == "unset polar" || s == "set nopolar") {
                p->polar = false;
                continue;
            }
            // Boxplots from libgretl suppress numeric x tics and place labels
            // manually.
            if (s == "set noxtics" || s == "unset xtics") {
                p->xtics_disabled = true;
                continue;
            }
            if (s == "set xtics") {
                p->xtics_disabled = false;
                continue;
            }
            // Also accept gretl's alternate `set noytics` spelling.
            if (s == "unset ytics" || s == "set noytics") {
                p->ytics_disabled = true;
                continue;
            }
            if (s == "set ytics") {
                p->ytics_disabled = false;
                continue;
            }
            // Reset text annotations before libgretl writes per-cell labels.
            if (s == "unset label") {
                p->labels.clear();
                continue;
            }
            if (starts(s, "set label ")) {
                // Accept libgretl labels in either data or graph coordinates.
                // Decimal commas in data coordinates must not be mistaken for
                // the separator between x and y (as used by boxplots).
                static const std::regex graph_label_re(
                    R"GP(^set label\s+(["'])(.*?)\1\s+at\s+graph\s+([^,\s]+)\s*,\s*graph\s+([^\s]+)(?:\s+(center|front|back))?\s*$)GP");
                static const std::regex point_label_re(
                    R"GP(^set label\s+(["'])(.*?)\1\s+at\s+([^,\s]+)\s*,\s*([^\s]+)\s+point\s+lt\s+([0-9]+)\s+pt\s+([0-9]+)\s+offset\s+([^,\s]+)\s*,\s*([^\s]+)\s*$)GP");
                static const std::regex data_label_re(
                    R"GP(^set label\s+(["'])(.*?)\1\s+at\s+(.+?)\s+center(?:\s+(?:front|back))*\s*$)GP");
                // PairPlot marks highlighted observations with numbered, empty
                // point labels using explicit size, type and RGB color.
                static const std::regex numbered_point_label_re(
                    R"GP(^set label\s+[0-9]+\s+(['"])(.*?)\1\s+at\s+([^,\s]+)\s*,\s*([^\s]+)\s+point\s+ps\s+([^\s]+)\s+pt\s+([0-9]+)\s+lt\s+(-?[0-9]+)\s+lc\s+(['"])(.*?)\8\s+lw\s+([^\s]+)(?:\s+(?:front|back))?\s*$)GP");
                std::smatch match;
                if (std::regex_match(s, match, numbered_point_label_re)) {
                    const auto x = number(match[3].str()), y = number(match[4].str());
                    const auto ps = number(match[5].str()), lw = number(match[10].str());
                    if (!x || !y || !ps || !lw || *ps <= 0 || *lw <= 0)
                        throw std::runtime_error("invalid point label: " + s);
                    Plot::Label label{match[2].str(), *x, *y, false, false};
                    label.point_marker = true;
                    label.point_size = *ps;
                    label.point_type = std::stoi(match[6].str());
                    label.point_line_type = std::stoi(match[7].str());
                    label.point_color = match[9].str();
                    p->labels.push_back(std::move(label));
                } else if (std::regex_match(s, match, graph_label_re)) {
                    const auto x = number(match[3].str()), y = number(match[4].str());
                    if (!x || !y)
                        throw std::runtime_error("invalid label coordinates: " + s);
                    // Layer keywords are accepted; Matplotlib draws graph
                    // annotations above the data by default.
                    p->labels.push_back({match[2].str(), *x, *y, true, match[5].str() == "center"});
                } else if (std::regex_match(s, match, point_label_re)) {
                    const auto x = number(match[3].str()), y = number(match[4].str());
                    const auto dx = number(match[7].str()), dy = number(match[8].str());
                    if (!x || !y || !dx || !dy)
                        throw std::runtime_error("invalid point label coordinates: " + s);
                    Plot::Label label{match[2].str(), *x, *y, false, false};
                    label.point_marker = true;
                    label.offset_x = *dx;
                    label.offset_y = *dy;
                    label.point_line_type = std::stoi(match[5].str());
                    p->labels.push_back(std::move(label));
                } else if (std::regex_match(s, match, data_label_re)) {
                    // Keep the original data-label coordinate parsing intact.
                    static const std::regex coords_re(R"GP(^([^,\s]+)\s*,\s*([^\s]+)$)GP");
                    std::smatch coords;
                    const std::string coordinates = match[3].str();
                    if (!std::regex_match(coordinates, coords, coords_re))
                        throw std::runtime_error("unsupported label: " + s);
                    const auto x = number(coords[1].str()), y = number(coords[2].str());
                    if (!x || !y)
                        throw std::runtime_error("invalid label coordinates: " + s);
                    p->labels.push_back({match[2].str(), *x, *y, false, true});
                } else {
                    throw std::runtime_error("unsupported label: " + s);
                }
                continue;
            }
            // Gnuplot margin values are measured in characters; `at screen`
            // instead specifies an absolute fraction of the full figure.
            // An omitted or negative character margin restores automatic layout.
            {
                static const std::regex margin_re(
                    R"(^(set|unset) ([lrbt])margin(?:\s+(.*))?$)");
                std::smatch margin_match;
                if (std::regex_match(s, margin_match, margin_re)) {
                    const std::string side = margin_match[2].str();
                    const auto index = std::string("lrbt").find(side);
                    auto& margin = p->margins[index];
                    std::string spec = trim(margin_match[3].str());
                    if (margin_match[1] == "unset" || spec.empty()) {
                        if (margin_match[1] == "unset" && !spec.empty())
                            throw std::runtime_error("invalid margin directive: " + s);
                        margin.reset();
                    } else {
                        const bool screen = starts(spec, "at screen ");
                        if (screen)
                            spec = trim(spec.substr(std::string("at screen ").size()));
                        const auto value = number(spec);
                        if (!value || !std::isfinite(*value) ||
                            (screen && (*value < 0.0 || *value > 1.0)))
                            throw std::runtime_error("invalid margin directive: " + s);
                        if (!screen && *value < 0.0)
                            margin.reset();
                        else
                            margin = Plot::Margin{*value, screen};
                    }
                    continue;
                }
            }
            // Approximate solid fill density; border styling uses matplotlib
            // defaults.
            if (starts(s, "set style fill ")) {
                const auto fill = trim(s.substr(std::string("set style fill ").size()));
                // Gnuplot's "transparent solid" uses the same density value.
                const bool transparent = starts(fill, "transparent solid");
                if (transparent || starts(fill, "solid")) {
                    std::istringstream args(trim(fill.substr(transparent ? 17 : 5)));
                    std::string density;
                    if (args >> density) {
                        if (density != "border" && density != "noborder") {
                            const auto value = number(density);
                            if (!value || *value < 0 || *value > 1)
                                throw std::runtime_error("invalid solid fill density: " + density);
                            p->fill_alpha = *value;
                        } else {
                            p->fill_alpha = 1.0;
                        }
                    } else {
                        p->fill_alpha = 1.0;
                    }
                    continue;
                }
                throw std::runtime_error("unsupported fill style: " + fill);
            }
            // Gnuplot boxwidth can be absolute or relative to the spacing of x
            // values.
            if (starts(s, "set boxwidth ")) {
                std::istringstream args(trim(s.substr(std::string("set boxwidth ").size())));
                std::string width, mode;
                args >> width >> mode;
                const auto value = number(width);
                if (!value || *value <= 0 ||
                    (!mode.empty() && mode != "relative" && mode != "absolute"))
                    throw std::runtime_error("unsupported boxwidth: " + s);
                p->box_width = *value;
                p->box_width_relative = (mode == "relative");
                continue;
            }
            if (s == "unset boxwidth") {
                p->box_width.reset();
                p->box_width_relative = false;
                continue;
            }
            if (s == "set nokey") {
                p->legend = false;
                continue;
            }
            if (starts(s, "set output ")) {
                d.output = quoted(s.substr(11));
                continue;
            }
            if (s == "set output" || s == "unset output")
                continue;
            if (starts(s, "set terminal ") || starts(s, "set term ")) {
                auto t = trim(s.substr(starts(s, "set terminal ") ? 13 : 9));
                std::istringstream ts(t);
                ts >> d.terminal;
                std::smatch m;
                if (std::regex_search(t, m, std::regex(R"(size\s+(\d+)\s*,\s*(\d+))"))) {
                    d.width = std::stoi(m[1]);
                    d.height = std::stoi(m[2]);
                }
                continue;
            }
            // Square axes are independent of the normalized size of a
            // multiplot panel; do not treat `square` as two numeric dimensions.
            if (s == "set size square") {
                p->square = true;
                p->equal_units = false;
                continue;
            }
            // Gnuplot's negative ratio -1 means identical lengths per x/y unit,
            // not necessarily a square axes box when ranges are different.
            if (s == "set size ratio -1") {
                p->equal_units = true;
                p->square = false;
                continue;
            }
            if (s == "set size noratio" || s == "set size nosquare") {
                p->equal_units = false;
                p->square = false;
                continue;
            }
            // The size and origin settings describe the next panel in normalized
            // figure units.
            if (starts(s, "set size ")) {
                std::smatch m;
                if (!std::regex_match(
                        s,
                        m,
                        std::regex(R"(^set size\s+([0-9]*\.?[0-9]+)\s*,\s*([0-9]*\.?[0-9]+)\s*$)")))
                    throw std::runtime_error("unsupported set size expression: " + s);
                p->size_x = std::stod(m[1]);
                p->size_y = std::stod(m[2]);
                if (p->size_x <= 0 || p->size_y <= 0) {
                    throw std::runtime_error("set size requires positive dimensions");
                }
                continue;
            }
            if (starts(s, "set origin ")) {
                std::smatch m;
                if (!std::regex_match(
                        s,
                        m,
                        std::regex(
                            R"(^set origin\s+(-?[0-9]*\.?[0-9]+)\s*,\s*(-?[0-9]*\.?[0-9]+)\s*$)")))
                    throw std::runtime_error("unsupported set origin expression: " + s);
                p->origin_x = std::stod(m[1]);
                p->origin_y = std::stod(m[2]);
                d.positioned_multiplot = true;
                continue;
            }
            if (s == "set multiplot next") {
                // A gridplot layout may skip cells without creating a drawable plot.
                // The current Plot is the empty placeholder for the next panel.
                if (!d.multiplot || !multiplot_layout || !p->series.empty() || !p->images.empty() ||
                    !p->contours.empty() || !p->surfaces.empty() || !p->matrix_labels.empty())
                    throw std::runtime_error("set multiplot next requires an empty layout panel");
                ++p->panel_index;
                continue;
            }
            if (s == "set multiplot" || starts(s, "set multiplot ")) {
                d.multiplot = true;
                std::smatch m;
                if (std::regex_search(s, m, std::regex(R"(layout\s+(\d+)\s*,\s*(\d+))"))) {
                    d.rows = std::stoi(m[1]);
                    d.cols = std::stoi(m[2]);
                    multiplot_layout = true;
                    if (d.rows <= 0 || d.cols <= 0)
                        throw std::runtime_error("multiplot layout requires positive dimensions");
                }
                // The multiplot title belongs to the figure, not the first axes.
                const auto title_pos = s.find(" title ");
                if (title_pos != s.npos)
                    d.multiplot_title = quoted(s.substr(title_pos + 7));
                continue;
            }
            if (s == "unset multiplot") {
                // A trailing configuration-only panel is not a drawable plot.
                if (d.multiplot && d.plots.size() > 1 && p->series.empty() && p->images.empty() &&
                    p->contours.empty() && p->surfaces.empty() && p->matrix_labels.empty()) {
                    d.plots.pop_back();
                    p = &d.plots.back();
                }
                continue;
            }
            if (s == "reset") {
                // Reset plot settings for the next panel without discarding
                // already parsed panels or document-level output settings.
                if (!p->series.empty() || !p->images.empty() || !p->contours.empty() ||
                    !p->surfaces.empty() || !p->matrix_labels.empty())
                    throw std::runtime_error("reset after plot data without a new panel");
                // Reset the gnuplot settings, but preserve the grid cell selected
                // by earlier plot commands and any set multiplot next skips.
                const auto panel_index = p->panel_index;
                *p = Plot{};
                p->panel_index = panel_index;
                named_line_styles.clear();
                // These flags are global Gnuplot state, not per-plot fields.
                contour_base = false;
                contour_view_map = false;
                contour_surface = true;
                pm3d_surface = false;
                dgrid3d.reset();
                contour_level_count = 0;
                contour_first_linetype = 0;
                contour_levels.clear();
                continue;
            }
            // Gnuplot's default data style applies when a plot item has no `with`
            // clause.
            if (s == "set style data histogram") {
                default_style = Style::Histograms;
                continue;
            }
            if (s == "set style data lines") {
                default_style = Style::Lines;
                continue;
            }
            if (s == "set style histogram rowstacked") {
                p->histogram_rowstacked = true;
                continue;
            }
            if (starts(s, "set encoding ") || starts(s, "set termoption ") ||
                starts(s, "set decimalsign ") || starts(s, "set datafile missing "))
                continue;
            if (text_command(s, "set title")) {
                p->title = quoted(s.substr(9));
                continue;
            }
            // Gnuplot's negative label forms clear the corresponding axis labels.
            if (s == "set noxlabel" || s == "unset xlabel") {
                p->x.label.clear();
                continue;
            }
            if (s == "set noylabel" || s == "unset ylabel") {
                p->y.label.clear();
                continue;
            }
            // Gnuplot 3D labels can specify font size and an explicit rotation.
            if (s == "set nozlabel" || s == "unset zlabel") {
                p->z.label.clear();
                p->z.label_font_size.reset();
                p->z.label_rotation.reset();
                continue;
            }
            if (s == "set noy2label" || s == "unset y2label") {
                p->y2.label.clear();
                continue;
            }
            // The periodogram uses a secondary frequency axis labelled in periods.
            if (s == "set nox2label" || s == "unset x2label") {
                p->x2.label.clear();
                continue;
            }
            if (text_command(s, "set xlabel")) {
                p->x.label = quoted(s.substr(10));
                continue;
            }
            if (text_command(s, "set ylabel")) {
                p->y.label = quoted(s.substr(10));
                continue;
            }
            if (text_command(s, "set zlabel")) {
                static const std::regex zlabel_re(
                    R"GP(^set zlabel\s+(['"])(.*?)\1(?:\s+font\s+(['"])(.*?)\3)?(?:\s+rotate\s+by\s+(-?[0-9]+(?:\.[0-9]+)?))?\s*$)GP");
                std::smatch match;
                if (!std::regex_match(s, match, zlabel_re))
                    throw std::runtime_error("unsupported zlabel directive: " + s);
                p->z.label = match[2].str();
                p->z.label_font_size.reset();
                p->z.label_rotation.reset();
                if (match[4].matched) {
                    // A font specification such as ",12" sets the label size.
                    static const std::regex font_size_re(R"(^(?:[^,]*),([0-9]+(?:\.[0-9]+)?)$)");
                    std::smatch font_match;
                    const std::string font = match[4].str();
                    if (!std::regex_match(font, font_match, font_size_re))
                        throw std::runtime_error("unsupported zlabel font: " + match[4].str());
                    p->z.label_font_size = std::stod(font_match[1].str());
                }
                if (match[5].matched)
                    p->z.label_rotation = std::stod(match[5].str());
                continue;
            }
            if (text_command(s, "set y2label")) {
                p->y2.label = quoted(s.substr(11));
                continue;
            }
            if (text_command(s, "set x2label")) {
                p->x2.label = quoted(s.substr(11));
                continue;
            }
            // The color axis is independent of x/y axes and applies to matrix images.
            if (starts(s, "set cbrange ")) {
                Axis range;
                set_range(range, s);
                p->colors.min = range.min;
                p->colors.max = range.max;
                continue;
            }
            // Only the RGB model is currently mapped to Matplotlib's RGB map.
            if (s == "set palette model RGB")
                continue;
            if (starts(s, "set cblabel")) {
                if (!text_command(s, "set cblabel"))
                    throw std::runtime_error("unsupported colorbox label: " + s);
                p->colors.label = quoted(s.substr(std::string("set cblabel").size()));
                continue;
            }
            if (s == "unset cblabel") {
                p->colors.label.clear();
                continue;
            }
            if (starts(s, "set palette defined ")) {
                p->colors.stops =
                    parse_palette(trim(s.substr(std::string("set palette defined ").size())));
                continue;
            }
            // Axis tic definitions and secondary-axis tic intervals are general plot
            // settings.
            // Gnuplot accepts tic lists both with and without a space before
            // the opening parenthesis, as emitted by the ghosts package.
            if (starts(s, "set xtics ") || starts(s, "set xtics(")) {
                p->xtics_disabled = false;
                parse_tics(p->x.tics, trim(s.substr(std::string("set xtics").size())));
                continue;
            }
            if (starts(s, "set ytics ") || starts(s, "set ytics(")) {
                p->ytics_disabled = false;
                parse_tics(p->y.tics, trim(s.substr(std::string("set ytics").size())));
                continue;
            }
            if (starts(s, "set x2tics ") || starts(s, "set x2tics(")) {
                parse_tics(p->x2.tics, trim(s.substr(std::string("set x2tics").size())));
                continue;
            }
            if (s == "set y2tics") {
                // Bare set y2tics enables the right-hand tic labels.
                p->y2.tics.enabled = true;
                continue;
            }
            if (starts(s, "set y2tics ") || starts(s, "set y2tics(")) {
                parse_tics(p->y2.tics, trim(s.substr(std::string("set y2tics").size())));
                continue;
            }
            if (starts(s, "set mxtics ")) {
                // Gretl emits these for quarterly/monthly time-series plots.
                auto n = number(s.substr(11));
                if (!n || *n < 0 || *n > 1000 || std::floor(*n) != *n) {
                    throw std::runtime_error("invalid mxtics");
                }
                p->mxtics = static_cast<int>(*n);
                continue;
            }
            if (starts(s, "set mx2tics ")) {
                auto n = number(s.substr(12));
                if (!n || *n < 0 || std::floor(*n) != *n) {
                    throw std::runtime_error("invalid mx2tics");
                }
                p->mx2tics = static_cast<int>(*n);
                continue;
            }
            if (starts(s, "set my2tics ")) {
                auto n = number(s.substr(12));
                if (!n || *n < 0 || std::floor(*n) != *n) {
                    throw std::runtime_error("invalid my2tics");
                }
                p->my2tics = static_cast<int>(*n);
                continue;
            }
            if (s == "set autoscale fix") {
                p->autoscale_fix = true;
                continue;
            }
            // A bare link uses the identity mapping between the primary and
            // secondary axes; explicit `via ... inverse ...` mappings remain
            // unsupported until the transformation is represented in the IR.
            if (s == "set link x" || s == "set link x2") {
                p->link_x = true;
                continue;
            }
            if (s == "set link y" || s == "set link y2") {
                p->link_y = true;
                continue;
            }
            // Identity links leave the secondary axis in the same coordinate system.
            if (s == "set link y2 via y inverse y") {
                continue;
            }
            // Other linked-axis transformations require an explicit mapping.
            if (starts(s, "set link "))
                throw std::runtime_error("unsupported linked-axis transformation: " + s);
            // Colorbox visibility and placement are independent of the palette.
            if (s == "unset colorbox") {
                p->colorbox = false;
                p->colorbox_position.reset();
                continue;
            }
            // Both commands restore the default-position palette colorbar.
            if (s == "set colorbox" || s == "set colorbox default") {
                p->colorbox = true;
                p->colorbox_position.reset();
                continue;
            }
            if (starts(s, "set colorbox user")) {
                // For 3D plots Gnuplot uses screen coordinates (fractions of the
                // whole figure) for the explicitly positioned colorbox.
                // The optional `screen` keyword also works in 2D.
                static const std::string real =
                    R"([+-]?(?:[0-9]+(?:\.[0-9]*)?|\.[0-9]+)(?:[eE][+-]?[0-9]+)?)";
                static const std::regex user_re(
                    "^set colorbox user origin(?: screen)?\\s+(" + real + ")\\s*,\\s*(" + real +
                    ")\\s+size(?: screen)?\\s+(" + real + ")\\s*,\\s*(" + real + ")\\s*$");
                std::smatch match;
                if (!std::regex_match(s, match, user_re))
                    throw std::runtime_error("unsupported colorbox geometry: " + s);
                std::array<double, 4> position{};
                for (size_t i = 0; i < position.size(); ++i)
                    position[i] = std::stod(match[i + 1].str());
                if (!std::all_of(position.begin(),
                                 position.end(),
                                 [](double value) { return std::isfinite(value); }) ||
                    position[2] <= 0 || position[3] <= 0)
                    throw std::runtime_error("invalid colorbox geometry: " + s);
                p->colorbox = true;
                p->colorbox_position = position;
                continue;
            }
            // Gnuplot can use splot to compute contours in a flat XY view.
            // These directives are meaningful together; never silently accept
            // an ordinary 3D surface as a two-dimensional contour plot.
            if (s == "set contour base") {
                contour_base = true;
                continue;
            }
            if (s == "unset contour") {
                contour_base = false;
                continue;
            }
            if (s == "unset surface") {
                contour_surface = false;
                continue;
            }
            if (s == "set surface") {
                contour_surface = true;
                continue;
            }
            if (s == "set view map") {
                contour_view_map = true;
                continue;
            }
            if (starts(s, "set view ")) {
                // Preserve Gnuplot's elevation and azimuth for true 3D plots.
                static const std::regex view_re(
                    R"(^set view\s+(-?[0-9]+(?:\.[0-9]+)?)\s*,\s*(-?[0-9]+(?:\.[0-9]+)?)\s*$)");
                std::smatch match;
                if (!std::regex_match(s, match, view_re))
                    throw std::runtime_error("unsupported 3D view: " + s);
                p->view_elevation = std::stod(match[1].str());
                p->view_azimuth = std::stod(match[2].str());
                contour_view_map = false;
                continue;
            }
            if (starts(s, "set pm3d")) {
                // Only a surface-positioned pm3d mode is meaningful in this
                // backend. Reject other modes rather than misrendering them.
                static const std::regex pm3d_re(
                    R"(^set pm3d(?: at s(?: hidden3d(?: [0-9]+)?)?)?\s*$)");
                if (!std::regex_match(s, pm3d_re))
                    throw std::runtime_error("unsupported pm3d mode: " + s);
                pm3d_surface = true;
                continue;
            }
            if (s == "unset pm3d") {
                pm3d_surface = false;
                continue;
            }
            // Standalone hidden3d removes hidden lines from wireframe splot
            // surfaces, but does not affect filled pm3d surfaces in Gnuplot.
            // The supported pm3d backend therefore needs no rendering change.
            if (s == "set hidden3d" || s == "unset hidden3d") {
                continue;
            }
            if (starts(s, "set dgrid3d ")) {
                // Named matrix blocks already provide a rectangular grid.
                // Validate this directive, but do not regrid matrix values.
                // For explicit XYZ points, retain the requested gridding parameters.
                static const std::regex dgrid_re(
                    R"(^set dgrid3d\s+([0-9]+)\s*,\s*([0-9]+)\s+qnorm\s+([0-9]+)\s*$)");
                std::smatch match;
                if (!std::regex_match(s, match, dgrid_re) || std::stoi(match[1].str()) < 2 ||
                    std::stoi(match[2].str()) < 2 || std::stoi(match[3].str()) < 1)
                    throw std::runtime_error("unsupported dgrid3d directive: " + s);
                dgrid3d = std::array<int, 3>{std::stoi(match[1].str()),
                                             std::stoi(match[2].str()),
                                             std::stoi(match[3].str())};
                continue;
            }
            if (s == "unset dgrid3d") {
                dgrid3d.reset();
                continue;
            }
            // `isosamples` controls function sampling in gnuplot. An explicit
            // matrix supplies its own grid, so there is nothing to resample.
            if (starts(s, "set isosamples ")) {
                static const std::regex isosamples_re(
                    R"(^set isosamples\s+([0-9]+)\s*(?:,\s*([0-9]+))?\s*$)");
                std::smatch match;
                if (!std::regex_match(s, match, isosamples_re) || std::stoi(match[1].str()) < 2 ||
                    (match[2].matched && std::stoi(match[2].str()) < 2))
                    throw std::runtime_error("invalid isosamples directive: " + s);
                continue;
            }
            // The heatmap package uses automatically spaced B-spline contour
            // levels; explicit levels are used when supplied by the caller.
            if (starts(s, "set cntrparam ")) {
                static const std::regex count_re(
                    R"(^set cntrparam\s+bspline\s+levels\s+([0-9]+)\s+firstlinetype\s+([0-9]+)\s*$)");
                static const std::regex discrete_re(
                    R"(^set cntrparam\s+levels\s+discrete\s+(.+)$)");
                std::smatch match;
                if (std::regex_match(s, match, count_re)) {
                    contour_level_count = std::stoi(match[1].str());
                    contour_first_linetype = std::stoi(match[2].str());
                    if (contour_level_count < 1 || contour_level_count > 256 ||
                        contour_first_linetype < 1 || contour_first_linetype > 256)
                        throw std::runtime_error("invalid contour level specification: " + s);
                    contour_levels.clear();
                } else if (std::regex_match(s, match, discrete_re)) {
                    contour_levels.clear();
                    for (const auto& part : split_specs(match[1].str())) {
                        auto value = number(trim(part));
                        if (!value || !std::isfinite(*value))
                            throw std::runtime_error("invalid discrete contour level: " + part);
                        contour_levels.push_back(*value);
                    }
                    if (contour_levels.empty() ||
                        !std::is_sorted(contour_levels.begin(), contour_levels.end()) ||
                        std::adjacent_find(contour_levels.begin(), contour_levels.end()) !=
                            contour_levels.end())
                        throw std::runtime_error("contour levels must be strictly increasing");
                    contour_level_count = 0;
                } else {
                    throw std::runtime_error("unsupported contour parameters: " + s);
                }
                continue;
            }
            if (starts(s, "splot ")) {
                // True 3D pm3d surfaces are represented separately from the
                // existing contour-map interpretation of `splot`.
                static const std::regex surface_plot_re(
                    R"(^splot\s+(\$[A-Za-z_][A-Za-z0-9_]*)\s+matrix\s+with\s+pm3d\s*$)");
                // Named XYZ sources select three 1-based datafile columns.
                // `palette` is the default pm3d coloring, so it needs no
                // additional rendering option after validating the syntax.
                static const std::regex xyz_plot_re(
                    R"(^splot\s+(\$[A-Za-z_][A-Za-z0-9_]*)\s+using\s+([1-9][0-9]*)\s*:\s*([1-9][0-9]*)\s*:\s*([1-9][0-9]*)\s+with\s+pm3d(?:\s+palette)?\s*$)");
                std::smatch surface_match;
                if (std::regex_match(s, surface_match, xyz_plot_re)) {
                    if (!pm3d_surface || contour_view_map)
                        throw std::runtime_error("unsupported 3D splot: " + s);
                    if (!p->series.empty() || !p->images.empty() || !p->contours.empty() ||
                        !p->matrix_labels.empty())
                        throw std::runtime_error("mixed 2D and 3D plot is unsupported");
                    const auto block = d.data_blocks.find(surface_match[1].str());
                    if (block == d.data_blocks.end())
                        throw std::runtime_error("unknown 3D data block: " +
                                                 surface_match[1].str());
                    SurfaceGrid surface;
                    const std::array<int, 3> columns{std::stoi(surface_match[2].str()),
                                                     std::stoi(surface_match[3].str()),
                                                     std::stoi(surface_match[4].str())};
                    surface.xyz_points = parse_xyz_points(block->second, columns);
                    surface.dgrid3d = dgrid3d;
                    p->surfaces.push_back(std::move(surface));
                    continue;
                }
                if (std::regex_match(s, surface_match, surface_plot_re)) {
                    if (!pm3d_surface || contour_view_map)
                        throw std::runtime_error("unsupported 3D splot: " + s);
                    // A mixed 2D/3D panel must not silently discard either layer.
                    if (!p->series.empty() || !p->images.empty() || !p->contours.empty() ||
                        !p->matrix_labels.empty())
                        throw std::runtime_error("mixed 2D and 3D plot is unsupported");
                    const auto block = d.data_blocks.find(surface_match[1].str());
                    if (block == d.data_blocks.end())
                        throw std::runtime_error("unknown 3D data block: " +
                                                 surface_match[1].str());
                    SurfaceGrid surface;
                    surface.matrix = parse_matrix(block->second);
                    if (surface.matrix.rows.size() < 2 || surface.matrix.rows[0].size() < 2)
                        throw std::runtime_error("3D surface requires at least 2 x 2 cells");
                    p->surfaces.push_back(std::move(surface));
                    continue;
                }
                // Only an explicitly requested flat contour view is supported
                // at this stage; pm3d and arbitrary 3D data remain errors.
                static const std::regex contour_plot_re(
                    R"(^splot\s+(\$[A-Za-z_][A-Za-z0-9_]*)\s+matrix\s+with\s+lines\s*$)");
                std::smatch match;
                if (!contour_base || !contour_view_map || contour_surface ||
                    !std::regex_match(s, match, contour_plot_re))
                    throw std::runtime_error("unsupported 3D splot: " + s);
                // Contour plots use 2D axes and cannot share a 3D surface panel.
                if (!p->surfaces.empty())
                    throw std::runtime_error("mixed 2D and 3D plot is unsupported");
                const auto block = d.data_blocks.find(match[1].str());
                if (block == d.data_blocks.end())
                    throw std::runtime_error("unknown contour data block: " + match[1].str());
                ContourGrid contour;
                contour.matrix = parse_matrix(block->second);
                if (contour.matrix.rows.size() < 2 || contour.matrix.rows[0].size() < 2)
                    throw std::runtime_error("contour matrix requires at least 2 x 2 cells");
                contour.level_count = contour_level_count > 0 ? contour_level_count : 7;
                contour.levels = contour_levels;
                // A contour's firstlinetype selects one style per level.
                // Retain the actual line colors already parsed globally.
                if (contour_first_linetype > 0) {
                    const size_t count = contour_levels.empty()
                                           ? static_cast<size_t>(contour.level_count)
                                           : contour_levels.size();
                    for (size_t k = 0; k < count; ++k) {
                        const size_t index = static_cast<size_t>(contour_first_linetype - 1) + k;
                        if (index < d.linetype_colors.size() && !d.linetype_colors[index].empty())
                            contour.colors.push_back(d.linetype_colors[index]);
                        else
                            break;
                        contour.widths.push_back(
                            index < d.linetype_widths.size() ? d.linetype_widths[index] : 1.5);
                    }
                }
                p->contours.push_back(std::move(contour));
                continue;
            }
            // Preserve time-axis directives; conversion belongs to the backend.
            if (s == "set xdata time") {
                p->xdata_time = true;
                continue;
            }
            if (s == "set xdata") {
                p->xdata_time = false;
                continue;
            }
            if (starts(s, "set timefmt ")) {
                std::smatch time_match;
                if (!std::regex_match(
                        s, time_match, std::regex(R"(^set timefmt\s+([\"'])(.*?)\1\s*$)")))
                    throw std::runtime_error("unsupported time format: " + s);
                p->timefmt = time_match[2].str();
                continue;
            }
            // Gnuplot's numeric axis formats are passed to matplotlib's formatter.
            if (starts(s, "set format ")) {
                std::smatch format_match;
                if (!std::regex_match(
                        s,
                        format_match,
                        std::regex(R"(^set format (x|y|x2|y2)\s+([\"'])(.*?)\2\s*$)")))
                    throw std::runtime_error("unsupported axis format: " + s);
                const auto axis = format_match[1].str();
                auto& tics = axis == "x"  ? p->x.tics
                           : axis == "y"  ? p->y.tics
                           : axis == "x2" ? p->x2.tics
                                          : p->y2.tics;
                tics.format = format_match[3].str();
                continue;
            }
            if (std::regex_search(s, std::regex(R"(^set xrange\s*\[)"))) {
                set_range(p->x, s);
                continue;
            }
            if (std::regex_search(s, std::regex(R"(^set zrange\s*\[)"))) {
                set_range(p->z, s);
                continue;
            }
            if (std::regex_search(s, std::regex(R"(^set yrange\s*\[)"))) {
                set_range(p->y, s);
                continue;
            }
            if (std::regex_search(s, std::regex(R"(^set y2range\s*\[)"))) {
                set_range(p->y2, s);
                continue;
            }
            // x2range defines an independent numeric scale for the top axis.
            if (std::regex_search(s, std::regex(R"(^set x2range\s*\[)"))) {
                set_range(p->x2, s);
                continue;
            }
            if (starts(s, "set grid")) {
                auto a = trim(s.substr(8));
                p->grid_x = a.empty() || a.find("xtics") != a.npos;
                p->grid_y = a.empty() || a.find("ytics") != a.npos;
                continue;
            }
            if (s == "unset grid") {
                p->grid_x = p->grid_y = false;
                continue;
            }
            if (starts(s, "set logscale ")) {
                // Gnuplot accepts an optional numeric base (default 10).
                // Preserve it in the IR so Matplotlib uses the same scale.
                std::istringstream input(trim(s.substr(13)));
                std::string axis_name, base_text, extra;
                input >> axis_name;
                double base = 10.0;
                if (input >> base_text) {
                    auto parsed = number(base_text);
                    if (!parsed || !std::isfinite(*parsed) || *parsed <= 1.0)
                        throw std::runtime_error("invalid logscale base: " + base_text);
                    base = *parsed;
                }
                if (input >> extra)
                    throw std::runtime_error("unsupported logscale specification: " + s);
                Axis* axis = axis_name == "x"  ? &p->x
                           : axis_name == "y"  ? &p->y
                           : axis_name == "y2" ? &p->y2
                                               : nullptr;
                if (!axis)
                    throw std::runtime_error("unsupported logscale axis");
                axis->log = true;
                axis->log_base = base;
                continue;
            }
            if (starts(s, "unset logscale")) {
                p->x.log = p->y.log = p->y2.log = false;
                continue;
            }
            if (s == "unset key") {
                p->legend = false;
                continue;
            }
            if (s == "set key" || starts(s, "set key ")) {
                p->legend = true;
                // Interpret gnuplot key placement independently of token order.
                // A bare "outside" defaults to the right of the plot.
                std::istringstream keyopts(s.substr(7));
                std::string token;
                p->legend_outside = false;
                p->legend_horizontal = "right";
                p->legend_vertical = "center";
                bool left = false, right = false, top = false, bottom = false;
                while (keyopts >> token) {
                    if (token == "outside" || token == "out")
                        p->legend_outside = true;
                    else if (token == "inside")
                        p->legend_outside = false;
                    else if (token == "left")
                        left = true;
                    else if (token == "right")
                        right = true;
                    else if (token == "top")
                        top = true;
                    else if (token == "bottom")
                        bottom = true;
                    else if (token == "center") {
                        // Center is the default for an unspecified dimension.
                    }
                }
                if (left)
                    p->legend_horizontal = "left";
                else if (right)
                    p->legend_horizontal = "right";
                if (top)
                    p->legend_vertical = "top";
                else if (bottom)
                    p->legend_vertical = "bottom";
                if (p->legend_outside) {
                    // A single top/bottom token requests an above/below legend.
                    if ((top || bottom) && !left && !right)
                        p->legend_horizontal = "center";
                } else if (left || right || top || bottom) {
                    p->legend_location = (top      ? "upper "
                                          : bottom ? "lower "
                                                   : "center ") +
                                         std::string(left    ? "left"
                                                     : right ? "right"
                                                             : "right");
                } else {
                    p->legend_location.clear();
                }
                continue;
            }
            if (starts(s, "plot ")) {
                if (!p->series.empty() || !p->images.empty() || !p->contours.empty() ||
                    !p->surfaces.empty()) {
                    const auto panel_index = p->panel_index;
                    d.plots.emplace_back();
                    p = &d.plots.back();
                    p->panel_index = panel_index + (d.multiplot ? 1 : 0);
                }
                std::string command = s.substr(5);
                while (!command.empty() && command.back() == '\\' && i + 1 < lines.size()) {
                    command.pop_back();
                    command += " " + trim(lines[++i]);
                }
                auto specs = split_specs(command, decimal_comma);
                // Parametric functions and data sources can coexist in a single
                // gnuplot plot command. CUB places ellipse pairs before the
                // original inline observations; the latter must still consume
                // their data rows through the ordinary series parser.
                static const std::regex parametric_x(R"(^x[0-9]*\(t\)$)");
                static const std::regex parametric_y(R"(^y[0-9]*\(t\)$)");
                bool has_parametric_pairs = false;
                bool has_ordinary_sources = false;
                for (size_t spec_index = 0; spec_index < specs.size(); ++spec_index) {
                    const auto& spec = specs[spec_index];
                    if (spec.empty()) {
                        continue;
                    }
                    std::istringstream st(spec);
                    std::string src;
                    st >> src;
                    // Keep numeric x coordinates paired with named y functions,
                    // as allowed by the existing restricted ellipse translator.
                    bool pair_start = parametric && std::regex_match(src, parametric_x);
                    if (parametric && !pair_start && number(src) && spec_index + 1 < specs.size()) {
                        std::istringstream next_stream(specs[spec_index + 1]);
                        std::string next_source;
                        next_stream >> next_source;
                        pair_start = std::regex_match(next_source, parametric_y);
                    }
                    if (pair_start) {
                        if (spec_index + 1 >= specs.size())
                            throw std::runtime_error("parametric plot requires x(t), y(t) pairs");
                        auto curves = ellipse_series(spec + ", " + specs[++spec_index],
                                                     parametric_functions,
                                                     trange,
                                                     d.linetype_colors);
                        p->series.insert(p->series.end(), curves.begin(), curves.end());
                        has_parametric_pairs = true;
                        continue;
                    }
                    if (parametric && std::regex_match(src, parametric_y))
                        throw std::runtime_error("parametric y(t) without preceding x(t): " + spec);
                    has_ordinary_sources = true;
                    // Empty quoted filenames reuse the preceding external plot source.
                    if (src == "''" || src == "\"\"") {
                        if (previous_file.empty())
                            throw std::runtime_error("no previous data file for empty source");
                        src = previous_file;
                    }
                    const bool file_source = src.size() >= 2 &&
                                             (src.front() == '\'' || src.front() == '"') &&
                                             src.back() == src.front();
                    if (file_source)
                        previous_file = src;
                    const bool inline_data = src == "'-'" || src == "\"-\"" || src == "-";
                    // Named sources reuse the same column interpretation as inline data.
                    // Named sources can contain either ordinary columns or a numeric
                    // matrix.
                    const std::vector<std::string>* named_rows = nullptr;
                    // A numeric plot source denotes a constant function, not a data file.
                    const auto constant_value = inline_data ? std::optional<double>{} : number(src);
                    // Parse the complete analytic source, not merely its first numeric
                    // token.
                    std::optional<std::string> analytic_value;
                    if (!inline_data) {
                        const auto title_pos = spec.find(" title ");
                        const auto notitle_pos = spec.find(" notitle");
                        const auto with_pos = spec.find(" with ");
                        const auto short_with_pos = spec.find(" w ");
                        size_t expr_end = spec.size();
                        for (const auto pos : {title_pos, notitle_pos, with_pos, short_with_pos})
                            if (pos != std::string::npos) {
                                expr_end = std::min(expr_end, pos);
                            }
                        const auto formula = trim(spec.substr(0, expr_end));
                        analytic_value = analytic_expression(formula, plot_scalars);
                        // gretl's FREQ gamma overlay is written as products of
                        // huge powers and gamma functions. Evaluate its log-density
                        // instead: direct exponentiation overflows for large alpha.
                        std::string compact;
                        for (const char c : formula)
                            if (!std::isspace(static_cast<unsigned char>(c)))
                                compact += c;
                        if (compact ==
                            "x**(alpha-1.0)*exp(-x/beta)/(exp(lgamma(alpha))*(beta**alpha))") {
                            const auto a = plot_scalars.find("alpha");
                            const auto b = plot_scalars.find("beta");
                            if (a != plot_scalars.end() && b != plot_scalars.end() &&
                                a->second > 0 && b->second > 0) {
                                std::ostringstream shape, scale;
                                shape << std::setprecision(17) << a->second;
                                scale << std::setprecision(17) << b->second;
                                const std::string alpha = "(" + shape.str() + ")";
                                const std::string beta = "(" + scale.str() + ")";
                                analytic_value =
                                    "(math.exp((" + alpha + "-1.0)*math.log(value)-value/" + beta +
                                    "-math.lgamma(" + alpha + ")-" + alpha + "*math.log(" + beta +
                                    ")) if value > 0 else " + "(0.0 if " + alpha + " > 1.0 else " +
                                    "(1.0/" + beta + " if " + alpha +
                                    " == 1.0 else float('inf'))))";
                            }
                        }
                    }
                    if (!inline_data && !constant_value && !analytic_value) {
                        auto found = d.data_blocks.find(src);
                        if (found != d.data_blocks.end()) {
                            named_rows = &found->second;
                        } else if (file_source) {
                            // Cache file rows: `''` must read exactly the same dataset.
                            auto cached = external_data.find(src);
                            if (cached == external_data.end()) {
                                const std::string filename = unquote(src);
                                std::ifstream file(filename);
                                if (!file)
                                    throw std::runtime_error("cannot open data file: " + filename);
                                std::vector<std::string> rows;
                                std::string record;
                                while (std::getline(file, record))
                                    rows.push_back(record);
                                cached = external_data.emplace(src, std::move(rows)).first;
                            }
                            named_rows = &cached->second;
                        } else {
                            throw std::runtime_error("unknown or unsupported data source: " + src);
                        }
                        if (spec.find(" matrix") != spec.npos) {
                            const auto style = keyword_value(spec, "with");
                            if (style == "image" && !using_offset(spec)) {
                                p->images.push_back(parse_matrix(*named_rows));
                            } else if (style == "labels") {
                                p->matrix_labels.push_back(parse_matrix_labels(spec, *named_rows));
                            } else {
                                throw std::runtime_error("unsupported matrix plot expression: " +
                                                         spec);
                            }
                            continue;
                        }
                    }
                    // A plot specification contains both presentation settings and a data
                    // source. Interpret its title, color and style independently: a
                    // single gnuplot plot command may mix lines, points and analytic
                    // functions.
                    Series ser;
                    ser.point_size = p->point_size;
                    ser.title = keyword_value(spec, "title");
                    if (ser.title.empty())
                        ser.title = keyword_value(spec, "t");
                    if (spec.find("notitle") != spec.npos)
                        ser.title = "";
                    ser.color = keyword_value(spec, "lc rgb");
                    if (ser.color.empty()) {
                        ser.color = keyword_value(spec, "linecolor rgb");
                    }
                    // Preserve explicit color selection before applying any palette
                    // fallback.
                    ser.explicit_color = !ser.color.empty();
                    // Gnuplot accepts both the long 'with' and abbreviated 'w' spelling.
                    // Unqualified data plots use set style data (points by default),
                    // while numeric and analytic functions default to lines.
                    auto style = keyword_value(spec, "with");
                    if (style.empty()) {
                        style = keyword_value(spec, "w");
                    }
                    if (style.empty()) {
                        ser.style =
                            (constant_value || analytic_value) ? Style::Lines : default_style;
                    } else if (style == "lines" || style == "l") {
                        ser.style = Style::Lines;
                    } else if (style == "hist" || style == "histogram" || style == "histograms") {
                        ser.style = Style::Histograms;
                    } else if (style == "points") {
                        ser.style = Style::Points;
                    } else if (style == "linespoints" || style == "lp") {
                        ser.style = Style::LinesPoints;
                    } else if (style == "impulses") {
                        ser.style = Style::Impulses;
                    } else if (style == "steps" || style == "fsteps") {
                        ser.style = Style::Steps;
                    } else if (style == "candlesticks") {
                        ser.style = Style::Candlesticks;
                    } else if (style == "boxes") {
                        ser.style = Style::Boxes;
                    } else if (style == "dots") {
                        ser.style = Style::Dots;
                    } else if (style == "filledcurve" || style == "filledcurves") {
                        ser.style = Style::FilledCurve;
                    } else if (style == "errorbars" || style == "yerrorbars" ||
                               style == "errorbar" || style == "yerrorbar") {
                        // The unqualified gnuplot errorbars style represents y errors.
                        ser.style = Style::ErrorBars;
                    } else {
                        throw std::runtime_error("unsupported plot style: " + style);
                    }
                    // Resolve explicit line type color through the document's style
                    // palette.
                    auto lt = keyword_value(spec, "lt");
                    if (lt.empty()) {
                        lt = keyword_value(spec, "linetype");
                    }
                    // Preserve the original line type for candlestick notch markers.
                    if (!lt.empty()) {
                        ser.line_type = std::stoi(lt);
                    }
                    // A selected line type is explicit even if its palette entry is
                    // absent.
                    ser.explicit_color = ser.explicit_color || !lt.empty();
                    // Preserve the gnuplot dash type for the rendering backend.
                    const std::string dt = keyword_value(spec, "dt");
                    if (!dt.empty()) {
                        try {
                            ser.dash_type = std::stoi(dt);
                        } catch (const std::exception&) {
                            // Keep the default solid line for unsupported dash
                            // specifications.
                        }
                    }
                    if (ser.color.empty() && !lt.empty()) {
                        int n = std::stoi(lt);
                        if (n > 0 && static_cast<size_t>(n) <= d.linetype_colors.size()) {
                            ser.color = d.linetype_colors[n - 1];
                        }
                    }
                    if (ser.color.empty() && !d.linetype_colors.empty() &&
                        !d.linetype_colors[0].empty()) {
                        ser.color = d.linetype_colors[0];
                    }
                    // Apply a named line style unless the series overrides its color
                    // or linewidth explicitly.
                    const auto ls = keyword_value(spec, "ls");
                    if (!ls.empty()) {
                        const auto it = named_line_styles.find(std::stoi(ls));
                        if (it == named_line_styles.end())
                            throw std::runtime_error("unknown named line style: " + ls);
                        if (!ser.explicit_color) {
                            ser.color = it->second.color;
                            ser.explicit_color = true;
                        }
                        ser.width = it->second.width;
                        ser.line_type = it->second.line_type;
                        if (ser.line_type == 0)
                            ser.dash_type = 3;
                    }
                    // Keep gnuplot line width on both ordinary series and constant
                    // functions.
                    auto lw = keyword_value(spec, "lw");
                    if (lw.empty()) {
                        lw = keyword_value(spec, "linewidth");
                    }
                    if (!lw.empty()) {
                        auto width = number(lw);
                        if (!width || *width <= 0) {
                            throw std::runtime_error("invalid line width: " + lw);
                        }
                        ser.width = *width;
                    }
                    if (spec.find("axes x1y2") != spec.npos)
                        ser.yaxis = 2;
                    if (analytic_value) {
                        // Analytic functions do not consume inline data rows.
                        ser.analytic_y = *analytic_value;
                        p->series.push_back(std::move(ser));
                        continue;
                    }
                    if (constant_value && !analytic_value) {
                        // A constant function does not consume subsequent inline data rows.
                        ser.constant_y = *constant_value;
                        p->series.push_back(std::move(ser));
                        continue;
                    }
                    const bool candle = ser.style == Style::Candlesticks;
                    const bool filled = ser.style == Style::FilledCurve;
                    const bool errorbars = ser.style == Style::ErrorBars;
                    const auto using_pos = using_offset(spec);
                    const bool has_using = using_pos.has_value();
                    // Error bars accept x:y:dy or x:y:ylow:yhigh, including
                    // explicit column expressions. Without `using`, the data
                    // row length determines whether a symmetric or asymmetric
                    // interval was supplied.
                    std::vector<std::string> error_fields;
                    if (errorbars && has_using) {
                        try {
                            error_fields = using_fields(spec, 4);
                        } catch (const std::runtime_error&) {
                            error_fields = using_fields(spec, 3);
                        }
                    }
                    // Arithmetic using expressions need per-row evaluation.
                    // Search only after 'using' (or 'u'): named sources such
                    // as $data are not column expressions.
                    const bool using_expr = !filled && !errorbars && has_using &&
                                            spec.find('$', *using_pos) != spec.npos;
                    const auto fields = candle     ? using_fields(spec, 5)
                                      : filled     ? using_fields(spec)
                                      : using_expr ? using_fields(spec, 2)
                                                   : std::vector<std::string>{};
                    auto xcol = (filled || using_expr || candle || errorbars)
                                  ? 1
                                  : using_col(spec, 1, true);
                    // Histograms use the row number as x; `using 2` selects only the
                    // height.
                    if (ser.style == Style::Histograms)
                        xcol = 0;
                    auto ycol = (filled || using_expr || candle) ? 2 : using_col(spec, 2, false);
                    size_t row = 0;
                    bool ended = false;
                    size_t named_index = 0;
                    while (true) {
                        std::string dat;
                        if (named_rows) {
                            if (named_index == named_rows->size()) {
                                ended = true;
                                break;
                            }
                            dat = trim((*named_rows)[named_index++]);
                        } else {
                            if (++i >= lines.size()) {
                                break;
                            }
                            dat = trim(lines[i]);
                            if (dat == "e" || dat == "end") {
                                ended = true;
                                break;
                            }
                        }
                        if (dat.empty() || dat[0] == '#')
                            continue;
                        std::istringstream ds(dat);
                        std::vector<std::string> cells;
                        std::string cell;
                        while (ds >> cell)
                            cells.push_back(cell);
                        if (cells.empty()) {
                            continue;
                        }
                        auto x = xcol == 0 ? std::optional<double>(static_cast<double>(row))
                                           : ((size_t)xcol <= cells.size() ? number(cells[xcol - 1])
                                                                           : std::nullopt);
                        auto y =
                            (size_t)ycol <= cells.size() ? number(cells[ycol - 1]) : std::nullopt;
                        if (candle) {
                            // Gnuplot candlesticks use x:open:low:high:close columns.
                            std::vector<std::optional<double>> values;
                            for (const auto& field : fields)
                                values.push_back(evaluate_using_field(field, cells));
                            if (std::all_of(values.begin(), values.end(), [](const auto& v) {
                                    return v.has_value();
                                })) {
                                ser.points.push_back({*values[0], *values[1]});
                                ser.candle_low.push_back(*values[2]);
                                ser.candle_high.push_back(*values[3]);
                                ser.candle_top.push_back(*values[4]);
                            }
                        } else if (filled) {
                            // Evaluate the two boundaries against the same data row.
                            auto fx = ColumnExpression(fields[0], cells).evaluate();
                            auto lower = ColumnExpression(fields[1], cells).evaluate();
                            auto upper = ColumnExpression(fields[2], cells).evaluate();
                            if (fx && lower && upper) {
                                ser.points.push_back({*fx, *lower});
                                ser.upper.push_back(*upper);
                            }
                        } else if (errorbars) {
                            // Retain the point estimate and the two error-bar bounds
                            // separately. Bootstrap quantile intervals need not contain
                            // the point estimate, so no symmetry is assumed.
                            std::vector<std::optional<double>> values;
                            if (error_fields.empty()) {
                                // Without `using`, read x:y:dy or x:y:ylow:yhigh.
                                for (size_t col = 0; col < std::min<size_t>(cells.size(), 4); ++col)
                                    values.push_back(number(cells[col]));
                            } else {
                                for (const auto& field : error_fields)
                                    values.push_back(evaluate_using_field(field, cells));
                            }
                            if (values.size() != 3 && values.size() != 4)
                                throw std::runtime_error("errorbars require 3 or 4 columns");
                            if (std::all_of(values.begin(), values.end(), [](const auto& v) {
                                    return v.has_value();
                                })) {
                                const double center = *values[1];
                                const double lo =
                                    values.size() == 3 ? center - std::abs(*values[2]) : *values[2];
                                const double hi =
                                    values.size() == 3 ? center + std::abs(*values[2]) : *values[3];
                                ser.points.push_back({*values[0], center});
                                ser.lower.push_back(lo);
                                ser.upper.push_back(hi);
                            }
                        } else if (using_expr) {
                            // Evaluate both using fields against the current data row.
                            auto fx = evaluate_using_field(fields[0], cells);
                            auto fy = evaluate_using_field(fields[1], cells);
                            if (fx && fy)
                                ser.points.push_back({*fx, *fy});
                        } else if (x && y) {
                            ser.points.push_back({*x, *y});
                        }
                        ++row;
                    }
                    if (!ended) {
                        throw std::runtime_error("missing inline data terminator 'e'");
                    }
                    p->series.push_back(std::move(ser));
                }
                // Preserve the prior no-legend behavior for purely parametric
                // ellipse plots. Mixed plots retain the ordinary data labels.
                if (has_parametric_pairs && !has_ordinary_sources)
                    p->legend = false;
                if (d.multiplot) {
                    // Gnuplot retains axis/style settings between panels. Each subsequent
                    // panel starts with a snapshot, while plotted data remain
                    // panel-local.
                    Plot next = *p;
                    // Advance the layout cursor after each plotted panel.
                    ++next.panel_index;
                    next.series.clear();
                    next.images.clear();
                    next.contours.clear();
                    next.surfaces.clear();
                    next.matrix_labels.clear();
                    d.plots.push_back(std::move(next));
                    p = &d.plots.back();
                }
                continue;
            }
            if (starts(s, "print ") || starts(s, "set print ") || s == "set print" ||
                starts(s, "gpver = "))
                continue;
            // handled by executor version probe
            if (starts(s, "set ") || starts(s, "unset ") || starts(s, "reset") ||
                starts(s, "load ") || starts(s, "splot ") || starts(s, "$"))
                throw std::runtime_error("unsupported gnuplot command: " + s);
            throw std::runtime_error("unrecognized command: " + s);
        } catch (const std::exception& e) {
            result.errors.push_back({source_lines[i], e.what()});
        }
    }
    return result;
}
} // namespace gptranslate
