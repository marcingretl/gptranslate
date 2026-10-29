#!/usr/bin/env python3
"""Verify that PARMA's step-sampled boxes collapse to actual bins."""
import ast
import pathlib
import subprocess
import sys

exe, gp, output = sys.argv[1:]
# The executor writes to the explicit output path and keeps stdout empty.
result = subprocess.run([exe, '--emit-python', '--output', output, gp], check=True,
                        capture_output=True, text=True)
assert not result.stdout
script = pathlib.Path(output).read_text()
assert '_box_centers.append' in script
assert 'width=_box_widths' in script
compile(script, output, 'exec')
# Evaluate only the generated histogram's data and coalescing logic.
module = ast.parse(script)
bar = next(i for i, n in enumerate(module.body)
           if isinstance(n, ast.Expr) and isinstance(n.value, ast.Call)
           and isinstance(n.value.func, ast.Attribute)
           and n.value.func.attr == 'bar')
start = next(i for i, n in enumerate(module.body[:bar])
             if isinstance(n, ast.Assign) and any(isinstance(t, ast.Name) and t.id == 'x'
                                                   for t in n.targets))
source = ast.Module(body=module.body[start:bar], type_ignores=[])
ns = {}
exec(compile(source, output, 'exec'), ns)
assert len(ns['x']) > 500
assert len(ns['_box_centers']) < 80, len(ns['_box_centers'])
assert max(ns['_box_widths']) > 10 * ns['_box_spacing']
