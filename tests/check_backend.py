#!/usr/bin/env python3
"""Check configurable Matplotlib headers without importing a GUI toolkit."""
import pathlib
import subprocess
import sys
import tempfile

executor, source = sys.argv[1:]

with tempfile.TemporaryDirectory() as tmp:
    output = pathlib.Path(tmp) / 'plot.py'

    def generate(*options):
        # Test the generated source without requiring Qt or a display server.
        subprocess.run([executor, *options, '--output', str(output), source],
                       check=True, capture_output=True, text=True)
        code = output.read_text(encoding='utf-8')
        compile(code, str(output), 'exec')
        return code

    default = generate()
    assert default.startswith('import matplotlib.pyplot as plt\n')
    assert "matplotlib.use('QtAgg')" not in default
    assert 'from PySide6 import QtCore' not in default
    assert default.endswith('plt.show()\n')

    header = ("from PySide6 import QtCore\n"
              "import matplotlib\n"
              "matplotlib.use('QtAgg')\n"
              "import matplotlib.pyplot as plt\n")
    qt_agg = generate('--backend=QtAgg')
    assert qt_agg.startswith(header)
    assert qt_agg.endswith('plt.show()\n')
    assert qt_agg[len(header):] == default[len('import matplotlib.pyplot as plt\n'):]

    # Backend selection and display suppression are independent options.
    qt_agg_no_show = generate('--backend=QtAgg', '--no-show')
    assert qt_agg_no_show.startswith(header)
    assert 'plt.show()' not in qt_agg_no_show
    assert qt_agg_no_show == qt_agg.removesuffix('plt.show()\n')

    # Other Matplotlib backends do not load Qt or PySide6.
    for name in ('TkAgg', 'Agg'):
        code = generate(f'--backend={name}')
        expected = f"import matplotlib\nmatplotlib.use('{name}')\nimport matplotlib.pyplot as plt\n"
        assert code.startswith(expected)
        assert 'from PySide6' not in code
        assert code.endswith('plt.show()\n')
        assert generate('--backend', name) == code

    # Explicitly empty backend preserves the default header exactly.
    assert generate('--backend=') == default
    assert generate('--backend', '') == default

    # An invalid option must fail instead of silently producing a default plot.
    bad = subprocess.run([executor, '--backend'], capture_output=True, text=True)
    assert bad.returncode != 0
    assert '--backend requires' in bad.stderr

    # Backend names must be escaped as Python strings, not interpolated as code.
    escaped = generate("--backend=x'\\y")
    assert "matplotlib.use('x\\'\\\\y')" in escaped
