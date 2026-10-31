#!/usr/bin/env python3
"""Check PySide6/QtAgg header generation without importing a GUI toolkit."""
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
    qtagg = generate('--qtagg')
    assert qtagg.startswith(header)
    assert qtagg.endswith('plt.show()\n')
    assert qtagg[len(header):] == default[len('import matplotlib.pyplot as plt\n'):]

    # Backend selection and display suppression are independent options.
    qtagg_no_show = generate('--qtagg', '--no-show')
    assert qtagg_no_show.startswith(header)
    assert 'plt.show()' not in qtagg_no_show
    assert qtagg_no_show == qtagg.removesuffix('plt.show()\n')
