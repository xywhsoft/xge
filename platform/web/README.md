# XGE Web/Emscripten Scaffold

This folder contains the browser shell used by the Web/Emscripten Sokol path.

Current scope:

- Uses WebGL2 through Emscripten and `SOKOL_GLES3`.
- Provides a stable `#canvas` element for Sokol.
- Captures stdout/stderr into an on-page log panel.
- Keeps the Web path separate from the mini program backend.

Build:

```sh
./build_web_exe.sh
```

Windows:

```bat
build_web_exe.bat
```

Run the generated output through a local HTTP server:

```sh
./serve_web_exe.sh
```

Windows:

```bat
serve_web_exe.bat
```

Then open `http://localhost:8000/xge_web.html`.

This scaffold is not marked as Web support complete until it builds with an Emscripten SDK and passes browser WebGL2 smoke testing.
