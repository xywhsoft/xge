# Unified Document example

The current Format menu acceptance is `document-v1-v1163-format-menu-acceptance.json`.
Normal and shared-runtime packages, external extraction and all three launchers
passed. Advanced scope remains frozen; physical IME/input and platform release
acceptance are still pending.

The current popup resize acceptance is `document-v1-v1152-find-popup-acceptance.json`.
Windows 21 and Linux 30 outer basic commands, three original consumers and
the normal/shared runtime packages, external extraction and all three root
launchers passed. Physical IME/input, DPI and other platform release acceptance
remain pending; advanced scope is frozen.

From the repository root, with GCC and windres on PATH:

```bat
call examples\xui_document\build.bat
build\xui_document.exe
```

A standalone Windows folder/ZIP is available through
[the first-version package entry](../../docs/XUI_DOCUMENT_RUN_PACKAGE.md).
Its root `Open Document.cmd` selects the correct working directory; the example
can run without the compiler, Python or source checkout.

The left editor owns a rich document. The middle editor and right preview share
one Markdown document. Source/Visual/Live MD switches the middle editor's projection;
all three modes use the same revision and undo history. Live MD shows the active
top-level container as source and renders the other containers. Ctrl+Z/Y, Ctrl+B/I,
Ctrl+A/C/X/V, selection, mouse wheel and Ctrl+wheel zoom are supported.
The drawing target, input viewport and views follow the native window size.
The toolbar wraps; below 900 pixels wide the three views stack vertically.
Resizing keeps document bindings, content, focus and undo history. Run
`python test_xui/build_document_demo_resize_test.py --with-find --with-demo-verification`
for the owned-window Windows/Linux checks; see
[native resize evidence and limits](../../docs/XUI_DOCUMENT_NATIVE_RESIZE.md).
The **Format** menu offers paragraphs, headings 1-3, lists, quotes, code blocks,
underline, strikethrough, inline code and clear formatting. Availability and
checked state follow the active editor mode and selection; unavailable items
are disabled. Source/Live modes can switch to Visual for structural formatting.
The menu restores editor focus and uses the same document history. See
[format menu scope and checks](../../docs/XUI_DOCUMENT_FORMAT_MENU.md).

The **Find** and **Replace** buttons open the existing native search window for
the last active editor. Ctrl+F/H also opens it. It provides previous/next,
match results, case/word/regex/selection options, current/all replacement and
the same document undo history. Escape closes the window and restores editor
focus. The open popup follows the host viewport when resized; query text and
input focus are preserved. The resize driver uses `--with-find` to verify this
with native Windows/Linux windows. Native Rich and Source/Visual/Live checks are available with:

```sh
python test_xui/build_document_demo_find_test.py --with-format
```

On Linux add `--runtime-dir /absolute/path/to/runtime` and optionally
`--cjk-font /absolute/path/to/local-font.ttc`. The test uses a private Xvfb
display with software OpenGL and does not access the desktop clipboard.

Click an editor, then **Open** or **Save as**. Rich documents use the native
`.xdoc` format; Markdown uses `.md` or `.txt` source files. Opening a file updates
the editor and its shared preview together. Failed reads keep the current
document. The demo asks before replacing unsaved changes. Save waits for any
pending document edit to finish; it reports a busy result rather than saving
an incomplete candidate. Images are host resource references; this demo does
not bundle external images into a document file or fetch remote resources.
The Windows demo uses Microsoft YaHei (or SimSun) as its Chinese font fallback.
Other hosts should supply a font covering their content through the existing
XGE fallback API before loading proxy fonts.

On Linux, build and run the same native example with:

```sh
python3 examples/xui_document/build_linux.py
./build/linux-native/xui_document
```

The Linux builder uses a separate `build/linux-native` runtime. Use
`--runtime-dir /absolute/path/to/runtime --output /absolute/path/to/example`
to compile against an existing normal/default library. Linux defaults to
DejaVu system fonts. `XUI_DOCUMENT_FONT`, `XUI_DOCUMENT_FONT_BOLD`,
`XUI_DOCUMENT_FONT_ITALIC` and `XUI_DOCUMENT_FONT_MONO` override font paths;
`XUI_DOCUMENT_CJK_FONT` supplies a local Chinese fallback. Configured missing
primary or CJK fonts fail with the path and error. Fonts are not bundled.
The same four verification flags passed on a private X11 display with Mesa
software OpenGL; this does not validate physical GPU performance, real IME,
desktop clipboard interoperability or Linux file dialogs. See
[native Linux evidence](../../docs/XUI_DOCUMENT_LINUX_NATIVE.md).

Native X11 UTF-8 text clipboard now has a separate authenticated private-display
regression entry. It verifies actual cross-process Copy/Cut/Paste in rich visual
and all Markdown modes, full 2 MiB transfers, Undo/Redo and rejection without
source loss beyond the X11 request limit (34 cases, 493 checks). Visual selections
containing images or other opaque objects return unsupported on this text-only
backend, preserving both the document and the previous clipboard; Markdown source
mode can copy their source syntax. Unsupported INCR/type transfers return an error
and preserve the document. The permanent entry is:

```sh
python3 test_xui/build_document_x11_clipboard_test.py
python3 test_xui/build_document_linux_basic_suite.py --with-x11-clipboard
```

This requires Xvfb, xauth and Mesa in addition to the normal Linux build tools;
it never connects to the invoking desktop display or clipboard. See
[scope and actual evidence](../../docs/XUI_DOCUMENT_X11_CLIPBOARD.md).

Linux now also has an authenticated private-display entry for the actual XUI
file and close dialogs, using the real XGE renderer and synthetic pointer/key
dispatch. It verifies 15 scenarios across Source/Visual/Live: save, reopen,
discard, cancel, clean close and pending-edit protection (2,682 checks).

```sh
python3 test_xui/build_document_linux_workflow_test.py
python3 test_xui/build_document_linux_basic_suite.py --with-x11-clipboard --with-native-workflows
```

Use `--runtime-dir` to reuse a current normal/default library, and optionally
`--cjk-font /absolute/path/to/local-font.ttc` on the workflow entry or the existing
`XUI_DOCUMENT_CJK_FONT` environment variable. Both optional suites together run
ten groups / 27 outer commands. This validates XUI dialogs, not a system file
picker or physical input. The optional native X11 queue extension is below. See
[Linux file/close evidence](../../docs/XUI_DOCUMENT_LINUX_WORKFLOWS.md).

To also test queued X11 Unicode, mouse cancel/discard, Ctrl+Z/Y and window-close
events in all three Markdown modes:

```sh
python3 test_xui/build_document_linux_workflow_test.py --with-native-input
python3 test_xui/build_document_linux_basic_suite.py --with-x11-clipboard --with-native-workflows --with-native-input
```

This adds three native runs (198 checks), making 23 internal workflow commands
and 108 captured images. It uses the driver's private authenticated display,
including temporary Unicode key mapping, and never connects to the user's
desktop. Cancel now restores the remembered editor focus, so subsequent input
and undo work. Windows WM_CHAR/WM_CLOSE also verifies this focus regression.
These synthetic native events do not validate a physical IME. See
[native queue and focus evidence](../../docs/XUI_DOCUMENT_X11_INPUT.md).


Closing the native window offers **Save / Discard / Cancel** for unsaved work.
Save asks for a destination for each dirty document and exits only after both
are saved. Cancelling either save dialog keeps the window open. Pending edits
also require this decision; Save waits for them to finish, and closing again
lets you cancel or discard. The close decision runs after queued input is
dispatched. File confirmation rechecks pending edits that started after the
dialog opened, returning busy while preserving the current document.

File cancellation, successful save/open, Escape and refusing discard also
restore the initiating editor's focus. The discard confirmation rechecks
pending work that began after its own prompt opened, returning busy and keeping
the current document and preview. The workflow now verifies subsequent Chinese
typing and keyboard undo through the focus route, without manually resetting
focus. See [file confirmation evidence](../../docs/XUI_DOCUMENT_FILE_CONFIRMATION.md).

For an automated real-XGE rendering check:

```bat
build\xui_document.exe --verify
```

This runs the GPU game-loop backend, hides the native window, renders three
frames in Source and three in Live MD, checks that each content panel contains
text pixels and the middle panel changes between modes, saves
`artifacts/xui-document-rebuild/native-smoke.png` and `native-live-smoke.png`, and exits with a failing status
on resource, rendering, pixel-validation or image-save failure. It does not
exercise a physical Windows IME, assistive technology or interactive dialogs.

`build\xui_document.exe --verify-image-placeholder` renders a missing Markdown
image through the same XGE backend, checks the placeholder background and border
pixels, and saves `artifacts/xui-document-rebuild/native-image-placeholder.png`.
`--verify-async` checks that a pending Markdown edit changes the source editor
before publication while the shared preview stays on the committed revision.
`--verify-files` saves and reopens both profiles using UTF-8 file paths, edits
Chinese text, checks Undo/Redo and dirty state, saves the changed content,
reopens it, and checks shared preview bindings and failed-open preservation.
It saves `artifacts/xui-document-rebuild/native-files.png`; generated temporary
document files are removed after success. This bypasses interactive file and
discard dialogs. The workflow entry below separately exercises those dialogs.

```bat
call test_xui\build_document_demo_workflow_test.bat
```

This runs the actual XUI file/confirmation dialogs with the native XGE renderer
and synthetic pointer/key input. It verifies all three Markdown modes, both
file formats, cancellation, refusing replacement, pending-edit preservation,
and five close outcomes (cancel, save both files, discard, clean, pending).
A separate native-window check posts Unicode WM_CHAR and WM_CLOSE messages
through the Windows queue using the normal VSYNC game loop, then verifies
cancel/reopen/discard and shared preview. Logs, screenshots and input hashes
are written to a new `build/document-workflow-*` directory. These checks
exercise the real dialogs and Windows message routing, but do not verify a
physical IME or manual keyboard/mouse operation.

The first usable version has a separate baseline entry:

```bat
call test_xui\build_document_basic_suite.bat
rem Include the optional Windows browser backend and its release export check:
call test_xui\build_document_basic_suite.bat --with-webview
```

Each run writes new logs and actual exit records under `build/document-basic-*`.
The current Windows baseline has 18 entries, including the permanent format-command regression. It checks
payload/caret capability, GFM word-internal strike, exact Markdown source,
Undo/Redo, mode preservation, and bounded allocation failures in direct and
fallback source updates. Run it separately with
`test_xui\build_document_format_command_test.bat`; see
[format-command evidence](../../docs/XUI_DOCUMENT_FORMAT_COMMANDS.md).
The native clipboard entry checks 32 cases across rich Visual and Markdown
Visual/Source/Live, preserving selection and history on failure or empty paste.
Actual native failure calls run only while another window holds the clipboard;
positive and empty cases use TestProxy without reading or overwriting user data.
The boundary-deletion entry now checks 594 combinations across 33 sources,
three dialects, three APIs and both selection directions. It preserves tabs,
trailing spaces, Unicode whitespace beside format delimiters and reference keys,
and checks the collapsed Cut caret using the unchanged semantic prefix.
Eight allocation-budget scans verify failure atomicity. Run these separately with
`test_xui\build_document_native_clipboard_test.bat` and
`test_xui\build_document_boundary_delete_test.bat`.
All 18 baseline entries and three existing text consumers passed on the current
runtime; see [basic correctness evidence](../../docs/XUI_DOCUMENT_CLIPBOARD.md).
The latest whitespace/caret extension passed without another product change;
see [whitespace evidence](../../docs/XUI_DOCUMENT_WHITESPACE.md).
The original full suite remains available for the later advanced phase.
See [first-version scope](../../docs/XUI_DOCUMENT_V1.md) for frozen features.

Linux/WSL has a separate foundation entry:

```sh
python3 test_xui/build_document_linux_basic_suite.py
```

It builds the normal shared library and runs the original Core, renderer,
editor, format, boundary-deletion, text-range, MessageList and CommonMark
regressions. The current source passed a fresh 137-unit/zero-reuse Linux build
and all eight groups. This uses TestProxy for UI behavior; native Linux GPU,
physical IME and system clipboard acceptance remain pending. See
[Linux commands and evidence](../../docs/XUI_DOCUMENT_LINUX_BASIC.md).

Before release, manually check Chinese IME composition/replacement, mouse
selection, keyboard copy/paste and Undo/Redo in both editors; switch Markdown
modes; open and save both file formats; cancel the discard dialog; and verify
that failed file opens leave the current content intact; close with unsaved
work and try Save, Discard and Cancel. Verify that cancelling a save keeps
the window open. Physical IME and cross-machine release acceptance remain
manual checks.

See [the API and delivery notes](../../docs/XUI_DOCUMENT.md) for ownership,
transactions, file saving, test commands and current limitations. In particular,
the advanced-object provider callbacks are extension points. The optional
Windows Document WebView2 provider typesets math and Mermaid and renders block
HTML statically; it is separate from the basic public WebView widget. Live MD
conservatively falls back to whole-source rendering when the current parser
cannot establish reliable container boundaries; renderer statistics report
this fallback.
