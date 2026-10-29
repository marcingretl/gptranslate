#!/usr/bin/env python3
"""Verify default contrast, opt-out, and CLI precedence over environment."""
import os
import pathlib
import subprocess
import sys
import tempfile

exe, gp = sys.argv[1:]
with tempfile.TemporaryDirectory() as tmp:
    def generate(extra=(), env=None):
        output = pathlib.Path(tmp) / 'result.py'
        subprocess.run([exe, *extra, '--output', str(output), gp], check=True,
                       capture_output=True, text=True, env=env)
        script = output.read_text()
        compile(script, str(output), 'exec')
        return script

    clean_env = os.environ.copy()
    clean_env.pop('GPTRANSLATE_AUTO_CONTRAST', None)
    contrast = generate(env=clean_env)
    assert "kw['color'] = '#B8C5D5'" in contrast
    assert "kw['color'] = '#C05A35'" in contrast

    original = generate(['--no-auto-contrast'], env=clean_env)
    assert "kw['color'] = '#d3d3d3'" in original
    assert "kw['color'] = '#B8C5D5'" not in original

    env_off = clean_env.copy()
    env_off['GPTRANSLATE_AUTO_CONTRAST'] = '0'
    assert generate(env=env_off) == original
    assert generate(['--auto-contrast'], env=env_off) == contrast

    env_on = clean_env.copy()
    env_on['GPTRANSLATE_AUTO_CONTRAST'] = '1'
    assert generate(env=env_on) == contrast
    assert generate(['--no-auto-contrast'], env=env_on) == original
