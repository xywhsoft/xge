# Native Document messages

This example embeds Rich and GFM Markdown Documents in MessageList using the
same native renderer as DocumentView. The third message and the scratch editor
share one Markdown Document. Typing, Source/Visual/Live changes and undo/redo
keep that binding. **Append reply** applies one local source transaction to the
second message; the list updates its height and the following row automatically.
Message text supports drag selection and Ctrl+C through the normal clipboard
backend. This is a local UI example; the scratch content lasts for this run.

Build against an existing normal XGE runtime from the repository root:

```powershell
python examples/xui_document_messages/build.py
build\xui_document_messages.exe
python test_xui/build_document_messages_test.py
```

On Linux:

```sh
python3 examples/xui_document_messages/build.py --runtime-dir /absolute/path/to/runtime
./build/linux-native/xui_document_messages
python3 test_xui/build_document_messages_test.py --runtime-dir /absolute/path/to/runtime --cjk-font /absolute/path/to/local-font.ttc
```

The builder needs GCC and a previously built normal XGE library; it does not
rebuild the library. `--output` chooses the executable. The test uses a new
output folder, captures six stages, and records actual exit codes and hashes.
On Windows it copies the requested DLL beside the example and runs with only
Windows system directories on PATH. On Linux it verifies the requested library
through `ldd` and starts an authenticated private Xvfb display with Mesa software
OpenGL; Xvfb and xauth are needed only for the native test.

Windows uses local Segoe UI/Consolas and a Chinese fallback. Linux uses DejaVu;
set `XUI_DOCUMENT_CJK_FONT` to an existing local font for Chinese content. No
fonts or network resources are downloaded. `--cjk-font` on the test supplies
that environment variable and records the font hash.

`--verify` on the executable exercises the actual message widgets and renderer:
bindings, hit/drag selection, shared Chinese input, mode/history retention,
updated-message text extraction, row geometry, scrolling and a narrower logical
viewport. The test extracts selected text into its own buffer and never calls
the OS clipboard. It does not validate a physical IME, physical input devices,
physical GPU, OS clipboard interoperability or native window resizing.

Windows' basic suite includes this native test. Linux's foundation exposes it
through `--with-native-messages`, so headless foundation checks do not require
Xvfb by default. See [scope and evidence](../../docs/XUI_DOCUMENT_NATIVE_MESSAGES.md).
