#!/usr/bin/env python3
"""Build the scene client with Qt 5 and native macOS library names."""
import argparse
import os
from pathlib import Path
import re
import subprocess
import sys

if sys.platform != 'darwin':
    raise SystemExit('This entry point is for macOS; use the existing platform build on Linux or Windows.')

root = Path(__file__).resolve().parent.parent
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--jobs', type=int, default=4)
parser.add_argument('--target', default='ausceneclient')
args = parser.parse_args()
if args.jobs < 1:
    parser.error('--jobs must be positive')
qmake = os.environ.get('QMAKE', '/opt/homebrew/opt/qt@5/bin/qmake')
prefix = os.environ.get('HOMEBREW_PREFIX', '/opt/homebrew')

def run(command, cwd):
    subprocess.run(command, cwd=cwd, check=True)

version = subprocess.check_output([qmake, '-query', 'QT_VERSION'], text=True).strip()
if not version.startswith('5.'):
    raise SystemExit('Qt 5 is required; set QMAKE to its qmake executable')
projects = {p.parent.name: p.read_text() for p in root.glob('*/reqt.opt')}
targets = {re.search(r'^TARGET\s*=\s*(\w+)', text, re.M)[1].lower(): name
           for name, text in projects.items()}
seen = set()

def build(name):
    if name in seen:
        return
    seen.add(name)
    text = projects[name]
    for lib in dict.fromkeys(re.findall(r'-l(Au\w+?)(?:_debug)?(?=\s|$)', text)):
        dependency = targets.get(lib.lower())
        if dependency and dependency != name:
            build(dependency)
    folder = root / name
    print('Building', name, flush=True)
    if name == 'augl':
        (folder / '.d').mkdir(exist_ok=True)
        header = folder / '.d/AuGLVer.h'
        if not header.exists() or header.read_text() != '#define AUGL41\n':
            header.write_text('#define AUGL41\n')
    pro = folder / (name + '.pro')
    run([qmake, '-project', '-o', pro.name], folder)
    with pro.open('a') as output:
        output.write('\nCONFIG -= release debug_and_release\nCONFIG += debug\n' + text + '\n')
        output.write(f'''\nCONFIG -= release debug_and_release
CONFIG += debug c++11
QT += widgets
INCLUDEPATH -= /usr/local/include /opt/local/include /usr/local/inc
INCLUDEPATH += {prefix}/opt/sdl2/include {prefix}/opt/sdl2/include/SDL2
LIBS += -L{prefix}/lib
QMAKE_SONAME_PREFIX = @rpath
QMAKE_RPATHDIR += $$absolute_path(../debug)
''')
    run([qmake, pro.name, 'MOC_DIR=.d/', 'OBJECTS_DIR=.d/', 'RCC_DIR=.d/'], folder)
    run(['make', '-j' + str(args.jobs)], folder)

if args.target not in projects:
    parser.error('unknown target: ' + args.target)
build(args.target)

if args.target == 'ausceneclient':
    run([sys.executable, str(root / 'tools/package-macos.py')], root)
