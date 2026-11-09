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
    assert 'plt.show()' not in default

    header = ("from PySide6 import QtCore\n"
              "import matplotlib\n"
              "matplotlib.use('QtAgg')\n"
              "import matplotlib.pyplot as plt\n")
    qtagg = generate('--backend=QtAgg')
    assert qtagg.startswith(header)
    assert 'plt.show()' not in qtagg
    assert qtagg[len(header):] == default[len('import matplotlib.pyplot as plt\n'):]

    # The QtAgg header remains unchanged when plt.show() is requested.
    qtagg_show = generate('--backend=QtAgg', '--add-show')
    assert qtagg_show.startswith(header)
    assert qtagg_show == qtagg + 'plt.show()\n'

    # The old Qt-only flag was removed in favor of --backend=NAME.
    invalid = subprocess.run([executor, '--qtagg'], capture_output=True, text=True)
    assert invalid.returncode != 0
    assert 'unknown argument: --qtagg' in invalid.stderr
