# Contributing

Contributions that improve compatibility with **real gretl/libgretl-generated gnuplot scripts** are welcome.

1. Open an issue describing the gretl command or package, expected plot, and observed translation failure. Attach a minimal `.gp` fixture and any required external data file, with permission to redistribute it.
2. Add or update a regression test in `tests/` and register it in `meson.build` when appropriate.
3. Preserve existing source comments and add clear comments for new non-obvious behavior.
4. Build and run `meson test -C build --print-errorlogs` before submitting a change.
5. Keep the parser, intermediate representation and Matplotlib backend responsibilities separate.

Code contributions are submitted under the project's GPL-3.0-or-later license unless otherwise agreed. Do not add code or test data copied from incompatible sources. Disclose substantial AI assistance in the pull request when relevant, and review all generated code for correctness and provenance.
