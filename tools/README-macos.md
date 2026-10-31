# macOS client

Build prerequisites: Python 3, Apple command-line developer tools, Qt 5.15,
SDL2 and (when using SDL2-compat) SDL3. These are needed only on the build Mac.

```sh
make mac-client
```

Use `MAC_JOBS=8` to change compiler parallelism. Override `QMAKE` and
`HOMEBREW_PREFIX` for installations outside Apple Silicon Homebrew defaults.
For a narrower development build: `python3 tools/build-macos.py --target auutil`.

The build produces the development executable under `ausceneclient/` and
libraries under `debug/`, then packages these distribution outputs at the
repository root, following the existing `Makefile.rel` distribution layout:

- `soh-macos-arm64/SoH.app`
- `soh-macos-arm64.zip`

To package existing compiled outputs without recompiling:

```sh
make mac-package
```

Share the ZIP. Recipients can extract `SoH.app` and move it to Applications;
Homebrew, Qt, Python and this source checkout are not required at runtime.
The package includes Qt frameworks/plugins, transitive third-party libraries,
SDL2 and SDL3, Hazeron resource libraries, textures, blueprints, game data,
a generated sound archive, and available dependency license notices.
The icon is generated from `ausceneclient/data/SoHIcon.ico`, the exact asset
referenced by the Windows EXE. Its sole 32x32 source image is scaled for larger
Mac icon sizes.

The current artifact is an arm64 debug build requiring macOS 26 or newer
because of the installed SDL dependencies. Packaging sets that minimum in
Info.plist, audits all linked dependencies, removes external runtime search
paths, and ad-hoc signs the bundle. Developer ID signing and Apple notarization
are not configured; this is not yet a notarized public release.
The source checkout lacks `ShroudOfGlory.wav`; packaging includes all available
sound files but cannot supply that missing music asset.

Existing Linux/Windows targets, `Makefile.rel`, and `reqt.opt` files remain
unchanged. Mac build overrides live in separate tooling. The macOS viewport
restore path accounts for Retina device pixel ratios after offscreen rendering,
keeping HUD text and icons at a consistent size. Other platforms retain their
existing viewport behavior. Mac mouse-look sampling also runs on the GUI thread
and recenters through Core Graphics without generating synthetic mouse events.
The existing sensitivity setting and other platforms' input paths are preserved.
Generated qmake projects/Makefiles are ignored outputs; use separate checkouts for concurrent
builds on different platforms.

Verification on 2026-10-30: the extracted ZIP passed deep/strict signature
verification, printed the client version, and opened the main menu from a
separate temporary directory under a sandbox denying reads of both
`/opt/homebrew` and the entire source workspace. This verifies local dependency
isolation. A local original-server session has also reached a rendered world;
manual testing confirmed stable Retina HUD sizing and normal mouse-look control.
Public-server compatibility and testing on another physical Mac remain
outstanding.
