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
#include "gptranslate/parser.hpp"
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
namespace fs = std::filesystem;
namespace {
// Read the complete gnuplot source, preserving its inline data.
std::string read_file(const fs::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("cannot open script: " + path.string());
    }
    return {std::istreambuf_iterator<char>(file), {}};
}
std::string read_stdin() {
    return {std::istreambuf_iterator<char>(std::cin), {}};
}
// Keep output generation separate from running the generated Python program.
void write_file(const fs::path& path, const std::string& data) {
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("cannot write file: " + path.string());
    }
    file << data;
    if (!file) {
        throw std::runtime_error("write failed: " + path.string());
    }
}
// Publish a fully written Python script in the queue using an atomic rename.
// Timestamp-prefixed names preserve creation order for sequential libgretl
// calls; the random suffix avoids collisions across processes and MPI workers.
fs::path enqueue_python(const fs::path& directory, const std::string& python) {
    if (!fs::is_directory(directory))
        throw std::runtime_error("plot queue directory does not exist: " + directory.string());
    std::random_device rd;
    for (unsigned int attempt = 0; attempt < 16; ++attempt) {
        const auto stamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
                               std::chrono::system_clock::now().time_since_epoch())
                               .count();
        std::ostringstream name;
        name << "plot_" << std::setw(20) << std::setfill('0') << stamp << '_' << std::hex << rd()
             << rd();
        const fs::path final = directory / (name.str() + ".py");
        const fs::path staging = directory / (name.str() + ".tmp");
        if (fs::exists(final) || fs::exists(staging)) {
            continue;
        }
        try {
            write_file(staging, python);
            fs::rename(staging, final);
            return final;
        } catch (...) {
            std::error_code ignored;
            fs::remove(staging, ignored);
            throw;
        }
    }
    throw std::runtime_error("unable to allocate unique plot queue filename");
}
} // namespace
int main(int argc, char** argv) {
    try {
        std::string input;
        std::string output;
        bool persist = false;
        // Let embedding applications control when figures are displayed.
        bool show = true;
        // An empty backend leaves Matplotlib's default backend selection unchanged.
        std::string backend_name;
        bool auto_contrast = true;
        bool contrast_cli_override = false;
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--auto-contrast") {
                // Explicitly enable contrast for combined plots.
                auto_contrast = true;
                contrast_cli_override = true;
                continue;
            } else if (arg == "--no-auto-contrast") {
                // Preserve the original source colors when requested.
                auto_contrast = false;
                contrast_cli_override = true;
                continue;
            } else if (arg == "-persist") {
                // libgretl uses -persist for interactive plots.
                persist = true;
                continue;
            } else if (arg == "--no-show") {
                show = false;
                continue;
            } else if (arg == "--backend") {
                // Pass a Matplotlib backend name through to the generated Python.
                if (++i >= argc || argv[i][0] == '-') {
                    throw std::runtime_error("--backend requires a backend name (or an empty string)");
                }
                backend_name = argv[i];
                continue;
            } else if (arg.rfind("--backend=", 0) == 0) {
                // --backend= without a value restores Matplotlib's own default.
                backend_name = arg.substr(10);
                continue;
            } else if (arg == "--emit-python") {
                // Accepted for compatibility with gptranslate 0.3.
                continue;
            } else if (arg == "--output") {
                if (++i >= argc) {
                    throw std::runtime_error("--output requires a .py filename");
                }
                output = argv[i];
            } else if (arg.rfind("--output=", 0) == 0) {
                output = arg.substr(9);
            } else if (arg == "--help") {
                std::cout << "gp-executor [--output=SCRIPT.py] "
                             "[--auto-contrast|--no-auto-contrast] [--no-show] "
                             "[--backend=NAME] [-persist] [script.gp]\n"
                             "Translate gnuplot source to Python/matplotlib; never execute "
                             "it.\n";
                return 0;
            } else if (arg == "--version") {
                std::cout << "gp-executor 0.12.0\n";
                return 0;
            } else if (!arg.empty() && arg[0] == '-') {
                throw std::runtime_error("unknown argument: " + arg);
            } else if (input.empty()) {
                input = arg;
            } else {
                throw std::runtime_error("multiple input files");
            }
        }
        const std::string source = input.empty() ? read_stdin() : read_file(input);
        // libgretl's version probe uses GPVAL_VERSION and 'set print'.
        // It must work even when matplotlib is not installed.
        if (source.find("GPVAL_VERSION") != std::string::npos &&
            source.find("set print") != std::string::npos) {
            const auto at = source.find("set print");
            const auto first = source.find_first_of("\"'", at);
            if (first == std::string::npos) {
                throw std::runtime_error("invalid version probe");
            }
            const auto last = source.find(source[first], first + 1);
            if (last == std::string::npos) {
                throw std::runtime_error("invalid version probe");
            }
            write_file(source.substr(first + 1, last - first - 1), "5.4.0\n");
            return 0;
        }
        // Preserve the behavior of the terminal capability probes from 0.3.
        if (input.empty() && source.find("plot ") == std::string::npos &&
            source.find("splot ") == std::string::npos) {
            if (source.find("set term ") != std::string::npos ||
                source.find("set terminal ") != std::string::npos) {
                if (source.find("wxt") != std::string::npos ||
                    source.find("x11") != std::string::npos ||
                    source.find(" qt") != std::string::npos ||
                    source.find("tikz") != std::string::npos)
                    return 1;
            }
            return 0;
        }
        auto result = gptranslate::parse(source);
        for (const auto& warning : result.warnings)
            std::cerr << "warning:" << warning.line << ": " << warning.message << '\n';
        for (const auto& error : result.errors)
            std::cerr << "error:" << error.line << ": " << error.message << '\n';
        if (!result.errors.empty()) {
            return 2;
        }
        // Auto-contrast is the default for combined plots. The environment
        // can override it, while an explicit CLI option takes precedence.
        const char* contrast_env = std::getenv("GPTRANSLATE_AUTO_CONTRAST");
        if (!contrast_cli_override && contrast_env != nullptr) {
            const std::string value(contrast_env);
            if (value == "0")
                auto_contrast = false;
            else if (value == "1")
                auto_contrast = true;
        }
        gptranslate::MatplotlibBackend backend(auto_contrast, show, backend_name);
        const std::string python = backend.generate(result.document);
        // Queue only libgretl's interactive invocations. Explicit --output
        // and standalone translation retain their previous behavior.
        const char* queue = std::getenv("GRETL4PY_PLOT_QUEUE");
        if (persist && output.empty() && queue != nullptr && *queue != '\0') {
            // Queue publication is sufficient; callers do not consume a path on
            // stdout.
            enqueue_python(fs::path(queue), python);
            return 0;
        }
        // The default output is next to the input, with the .py extension.
        // No PNG is produced by gp-executor.
        const fs::path target =
            output.empty()
                ? (input.empty() ? fs::path("plot.py") : fs::path(input).replace_extension(".py"))
                : fs::path(output);
        // Keep stdout free of informational paths in normal translation mode.
        write_file(target, python);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "gp-executor: " << error.what() << '\n';
        return 1;
    }
}
