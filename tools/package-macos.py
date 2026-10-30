#!/usr/bin/env python3
"""Create a relocatable Mac client beside the existing distribution outputs."""
import os
from pathlib import Path
import platform
import plistlib
import shutil
import subprocess
import sys
import tempfile
import struct

if sys.platform != 'darwin':
    raise SystemExit('Mac packaging requires macOS')
root = Path(__file__).resolve().parent.parent
qmake = os.environ.get('QMAKE', '/opt/homebrew/opt/qt@5/bin/qmake')
prefix = Path(os.environ.get('HOMEBREW_PREFIX', '/opt/homebrew'))

def run(*args, **kwargs):
    subprocess.run([str(a) for a in args], check=True, **kwargs)

def copy_file(source, destination):
    copied = Path(shutil.copy2(source, destination))
    copied.chmod(copied.stat().st_mode | 0o200)
    return copied

def query(key):
    return Path(subprocess.check_output([qmake, '-query', key], text=True).strip())

def dependencies(binary):
    linked = [line.strip().split(' (')[0] for line in subprocess.check_output(
        ['otool', '-L', str(binary)], text=True).splitlines()[1:]]
    identity = subprocess.check_output(['otool', '-D', str(binary)], text=True).splitlines()[1:]
    return [dep for dep in linked if dep not in identity]

def machos():
    result = []
    for path in app.rglob('*'):
        if path.is_file() and not path.is_symlink():
            with path.open('rb') as binary:
                if binary.read(4) in (b'\xcf\xfa\xed\xfe', b'\xce\xfa\xed\xfe', b'\xca\xfe\xba\xbe'):
                    result.append(path)
    return result

source = root / 'ausceneclient/AuSceneClient_debug.app'
if not source.exists():
    raise SystemExit('Build the scene client first with make mac-client')
output = root / ('soh-macos-' + platform.machine())
staging = tempfile.TemporaryDirectory(prefix='soh-package-')
app = Path(staging.name) / 'SoH.app'
output.mkdir(exist_ok=True)
shutil.copytree(source, app, symlinks=True)
contents = app / 'Contents'
frameworks = contents / 'Frameworks'
resources = contents / 'Resources'
frameworks.mkdir(exist_ok=True)
resources.mkdir(exist_ok=True)
exe = contents / 'MacOS/SoH'
(contents / 'MacOS/AuSceneClient_debug').rename(exe)
with (contents / 'Info.plist').open('rb') as f:
    info = plistlib.load(f)
info.update(CFBundleExecutable='SoH', CFBundleName='Shores of Hazeron',
            CFBundleIdentifier='net.hazeron.soh', CFBundleIconFile='SoH.icns',
            CFBundleShortVersionString='7.0', CFBundleVersion='7.0',
            LSMinimumSystemVersion='26.0')
with (contents / 'Info.plist').open('wb') as f:
    plistlib.dump(info, f)
# Use the same artwork referenced by the Windows executable resource.
with tempfile.TemporaryDirectory(prefix='soh-icon-') as temporary:
    iconset = Path(temporary) / 'SoH.iconset'
    iconset.mkdir()
    png = Path(temporary) / 'windows-icon.png'
    run('sips', '-s', 'format', 'png', root / 'ausceneclient/data/SoHIcon.ico', '--out', png)
    chunks = []
    for size, kind in [(16, b'icp4'), (32, b'icp5'), (64, b'icp6'), (128, b'ic07'),
                       (256, b'ic08'), (512, b'ic09'), (1024, b'ic10')]:
        image = iconset / f'icon_{size}.png'
        run('sips', '-z', size, size, png, '--out', image)
        data = image.read_bytes()
        chunks.append(kind + struct.pack('>I', len(data) + 8) + data)
    payload = b''.join(chunks)
    (resources / 'SoH.icns').write_bytes(b'icns' + struct.pack('>I', len(payload) + 8) + payload)
copy_file(root / 'GD', resources)
shutil.copytree(root / 'Blueprints', resources / 'Blueprints', dirs_exist_ok=True)
tex = resources / 'tex'
tex.mkdir(exist_ok=True)
for image in (root / 'autexture/data').glob('*.png'):
    copy_file(image, tex)
for library in (root / 'debug').glob('libAu*.7.dylib'):
    copy_file(library, frameworks / library.name)
# SDL2-compat dlopens SDL3 via @loader_path/libSDL3.dylib.
for name, library in [('sdl2', 'libSDL2-2.0.0.dylib'), ('sdl3', 'libSDL3.dylib')]:
    copy_file(prefix / 'opt' / name / 'lib' / library, frameworks / library)
for binary in [exe, *frameworks.glob('*.dylib')]:
    for dep in dependencies(binary):
        name = Path(dep).name
        if (frameworks / name).exists():
            run('install_name_tool', '-change', dep, '@rpath/' + name, binary)
    if binary != exe:
        run('install_name_tool', '-id', '@rpath/' + binary.name, binary)
# Build the archive utility outside the tracked hashpack project.
helper = root / 'debug/mac-package-tools'
helper.mkdir(exist_ok=True)
(helper / 'hashpack.pro').write_text('QT = core\nCONFIG += console c++11\nCONFIG -= app_bundle\n'
    'TARGET = hashpack\nSOURCES = "' + str(root / 'hashpack/main.cpp') + '"\n')
run(qmake, 'hashpack.pro', cwd=helper)
run('make', '-j4', cwd=helper)
run(helper / 'hashpack', '-c', resources / 'sfx.har', '*.wav', cwd=root / 'auglobal/sfx')
(contents / 'MacOS/tex').symlink_to('../Resources/tex')
(contents / 'MacOS/sfx.har').symlink_to('../Resources/sfx.har')
run(query('QT_INSTALL_BINS') / 'macdeployqt', app, '-always-overwrite', '-appstore-compliant')
# macdeployqt can omit dependencies of non-Qt libraries. Close that graph too.
while True:
    added = False
    for binary in machos():
        for dep in dependencies(binary):
            if dep.startswith(('/System/Library/', '/usr/lib/')):
                continue
            name = Path(dep).name
            if dep.startswith('@rpath/') and (frameworks / dep[7:]).exists():
                continue
            if dep.startswith(('@loader_path/', '@executable_path/')):
                continue
            installed = Path(dep) if dep.startswith('/') else prefix / 'lib' / name
            destination = frameworks / name
            if not destination.exists():
                if not installed.exists():
                    raise SystemExit(f'Cannot deploy dependency: {dep}')
                copy_file(installed, destination)
                run('install_name_tool', '-id', '@rpath/' + name, destination)
                added = True
            run('install_name_tool', '-change', dep, '@rpath/' + name, binary)
    if not added:
        break
# Remove development search paths so dyld can only use bundled dependencies.
for binary in machos():
    load = subprocess.check_output(['otool', '-l', str(binary)], text=True).splitlines()
    for i, line in enumerate(load):
        if line.strip() == 'cmd LC_RPATH':
            path = load[i + 2].strip().split('path ', 1)[1].split(' (')[0]
            if path.startswith('/'):
                run('install_name_tool', '-delete_rpath', path, binary)
# Reject undeployed link dependencies, including transitive Qt/plugin dependencies.
for binary in machos():
    for dep in dependencies(binary):
        if dep.startswith(('/System/Library/', '/usr/lib/')):
            continue
        if dep.startswith('@rpath/'):
            resolved = frameworks / dep[len('@rpath/'):]
        elif dep.startswith('@loader_path/'):
            resolved = binary.parent / dep[len('@loader_path/'):]
        elif dep.startswith('@executable_path/'):
            resolved = exe.parent / dep[len('@executable_path/'):]
        else:
            raise SystemExit(f'External dependency in {binary}: {dep}')
        if not resolved.exists():
            raise SystemExit(f'Missing bundled dependency in {binary}: {dep}')
licenses = resources / 'Licenses'
licenses.mkdir(exist_ok=True)
copy_file(root / 'LICENSE', licenses / 'Hazeron-LICENSE')
origins = {'Qt': query('QT_INSTALL_PREFIX'), 'SDL2': prefix / 'opt/sdl2',
           'SDL3': prefix / 'opt/sdl3'}
for library in frameworks.glob('*.dylib'):
    installed = prefix / 'lib' / library.name
    if installed.exists():
        resolved = installed.resolve()
        if 'Cellar' in resolved.parts:
            index = resolved.parts.index('Cellar')
            origins[resolved.parts[index + 1]] = Path(*resolved.parts[:index + 3])
for name, directory in origins.items():
    for license_file in list(directory.glob('LICENSE*')) + list(directory.glob('COPYING*')):
        if license_file.is_file():
            copy_file(license_file, licenses / (name + '-' + license_file.name))
for path in app.rglob('*'):
    if not path.is_symlink():
        path.chmod(path.stat().st_mode | 0o200)
run('xattr', '-cr', app)
for binary in machos():
    if binary != exe:
        run('codesign', '--force', '--sign', '-', binary)
for framework in frameworks.glob('*.framework'):
    run('codesign', '--force', '--sign', '-', framework)
run('codesign', '--force', '--sign', '-', app)
run('codesign', '--verify', '--deep', '--strict', app)
archive = root / (output.name + '.zip')
if archive.exists():
    archive.unlink()
run('ditto', '-c', '-k', '--sequesterRsrc', '--keepParent', app, archive)
destination = output / 'SoH.app'
if destination.exists():
    shutil.rmtree(destination)
shutil.copytree(app, destination, symlinks=True, copy_function=shutil.copy)
run('xattr', '-cr', destination)
run('codesign', '--verify', '--deep', '--strict', destination)
staging.cleanup()
print('Packaged:', destination, '\nArchive:', archive)
