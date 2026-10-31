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
#include "plot.hpp"
#include <string>
namespace gptranslate {
class Backend {
  public:
    virtual ~Backend() = default;
    virtual std::string generate(const Document& document) const = 0;
};
class MatplotlibBackend final : public Backend {
  public:
    // Readability mode is enabled by default for combined plots.
    explicit MatplotlibBackend(bool auto_contrast = true, bool show = true, bool qtagg = false)
        : auto_contrast_(auto_contrast), show_(show), qtagg_(qtagg) {
    }
    std::string generate(const Document& document) const override;

  private:
    bool auto_contrast_ = false;
    // Emit plt.show() only for scripts that manage their own display.
    bool show_ = true;
    // Select PySide6/QtAgg in the generated Python only when explicitly requested.
    bool qtagg_ = false;
};
} // namespace gptranslate
