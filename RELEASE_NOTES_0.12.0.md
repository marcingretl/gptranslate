# gptranslate 0.12.0 — first public release

gptranslate is an experimental C++17 gnuplot-to-Python/Matplotlib translator focused on the plotting scripts produced by gretl/libgretl and selected gretl function packages.

This first public release includes a parser, intermediate plot representation, Matplotlib code generator, a command-line translator (`gp-executor`), gretl4py integration facilities, and a growing regression-test suite.

**Limitations:** This is not a complete gnuplot implementation. Full `gridplot` compatibility is not yet available, and visual equivalence is not guaranteed for every plot.

**Build:** `meson setup build && meson compile -C build && meson test -C build --print-errorlogs`.

**License:** GNU GPL v3 or later. Copyright (C) 2026 Marcin Błażejowski. Developed with substantial assistance from OpenAI ChatGPT.
