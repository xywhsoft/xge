# Standalone Windows WebView example

This example hosts one independent webpage. It uses only public XUI APIs for
local HTML, URL navigation, focus, zoom, layout and destruction. Its initial
page is self-contained and offers editable fields, selection, links and a
collapsible section. The toolbar supports a URL, Go, Local HTML, Focus and zoom.
It does not depend on Document or MessageList and has no application message bridge.

Build the optional WebView2 runtime with the existing normal builder, then the
example. Run from the repository root, with GCC on PATH:

```powershell
$env:WEBVIEW2_SDK_DIR = 'D:\path\to\extracted\WebView2SDK'
$env:XUI_ENABLE_WEBVIEW2 = '1'
python tools/build/build.py default build/webview-basic
python examples/xui/xui_webview/build.py
build\webview-basic\xui_webview.exe
```

The builder copies `WebView2Loader.dll` beside `xge.dll`. The Microsoft WebView2
Runtime must also be installed on the machine; the loader is not the browser
runtime. The example reports initialization/navigation failures in its status
line and console. No browser engine or fonts are downloaded by the example.
`XUI_WEBVIEW_FONT` can select another existing local toolbar font.

Normal runs use `%LOCALAPPDATA%\XGE\WebViewDemo` as their dedicated browser profile.
The automated check uses a separate process-specific directory under `%TEMP%`:

```powershell
build\webview-basic\xui_webview.exe --verify
```

It loads local HTML, clicks the actual Go and Local HTML toolbar buttons,
checks three completed navigations, then exercises toolbar zoom/focus and closes
the browser. This is synthetic UI/API verification, not physical keyboard/IME
or a pixel-level acceptance of the native browser compositor. It writes
`webview-toolbar.png`, which captures only the XGE toolbar above the native
browser child surface.

`--runtime-dir` selects another already built runtime; `--output` selects the
executable path. The executable needs that runtime's `xge.dll` and loader beside
it or on the process DLL search path. The current backend is Windows only.

For a separate runnable folder/ZIP with the Document example, see
[the first-version package](../../docs/guide/XUI_DOCUMENT_RUN_PACKAGE.md).
