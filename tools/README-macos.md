# macOS development build

Requires Python 3, Apple command-line developer tools, Qt 5.15 and SDL2.
From the repository root:

```sh
make mac-client
```

Use `make mac-client MAC_JOBS=8` to change the number of compiler jobs.

The script defaults to Apple Silicon Homebrew paths. Override `QMAKE` and
`HOMEBREW_PREFIX` for another installation. It generates ignored in-source
qmake projects/Makefiles, selects OpenGL 4.1, and builds the scene client's
library dependencies before the application. Output libraries go in `debug/`.
Use `--target auutil` (or another project directory) for a narrower build.

This is an incremental development build, not a distribution packaging tool.
The full arm64 debug build completed on 2026-10-30 with Qt 5.15.19.
The executable reports `AuSceneClient Version 7b2, Oct 30 2026` and launches
to the Shores of Hazeron age-confirmation screen. Login, gameplay graphics,
audio and server compatibility have not been verified.

Launch with:

```sh
open ausceneclient/AuSceneClient_debug.app
```

The final preparation step fixes resource dylib install names to `@rpath`,
clears Finder metadata from generated binaries and ad-hoc signs the outputs.
For existing compiled outputs, run `python3 tools/prepare-macos-app.py`.
The app still depends on this checkout's `debug/` directory and local Homebrew
libraries. This is not a portable distribution. The installed SDL2 compatibility
library was built for macOS 26; older OS support needs separately built dependencies.
Startup logs a missing `ShroudOfGlory.wav` music asset, which is absent from the
checkout. It does not prevent the initial window from opening.

The root Makefile's `mac-client` target delegates to the Mac-only build script.
Existing Makefile targets, `Makefile.rel`, `reqt.opt`, Windows scripts and C++
sources remain unchanged. The generated qmake files are ignored build outputs. Use separate
checkouts when building different platforms concurrently.
