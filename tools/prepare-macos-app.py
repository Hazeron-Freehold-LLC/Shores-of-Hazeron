#!/usr/bin/env python3
"""Repair development dylib paths and ad-hoc sign the local Mac app."""
from pathlib import Path
import subprocess

root = Path(__file__).resolve().parent.parent
app = root / 'ausceneclient/AuSceneClient_debug.app'
executable = app / 'Contents/MacOS/AuSceneClient_debug'
libraries = list((root / 'debug').glob('*.7.0.dylib'))

def run(*args):
    subprocess.run(args, check=True)

for library in libraries:
    run('install_name_tool', '-id', '@rpath/' + library.name.replace('.7.0.', '.7.'), str(library))
for binary in [*libraries, executable]:
    for line in subprocess.check_output(['otool', '-L', str(binary)], text=True).splitlines()[1:]:
        dependency = line.strip().split(' (')[0]
        if dependency.startswith('libAu'):
            run('install_name_tool', '-change', dependency, '@rpath/' + dependency, str(binary))
# Finder metadata on generated outputs can prevent ad-hoc signing.
for library in libraries:
    run('xattr', '-c', str(library))
    run('codesign', '--force', '--sign', '-', str(library))
run('xattr', '-cr', str(app))
run('codesign', '--force', '--sign', '-', str(app))
