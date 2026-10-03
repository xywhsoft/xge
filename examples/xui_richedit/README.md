# Unified rich text and Markdown example

The former RichEdit example entry point now builds the shared [Document demo](../xui_document/README.md). It shows a rich text editor beside a Markdown editor and a second view of the same Markdown document. Source, visual, and live Markdown modes share one history.

From the repository root:

```bat
call examples\xui_richedit\build.bat
build\xui_richedit.exe
```

`build\xui_richedit.exe --verify` checks native rendering; `--verify-async` checks that pending source input leaves the shared preview unchanged until publication. The example uses `xui_document.h` and `xui_document_ui.h`; it does not call the old RichDocument or RichEdit interfaces.
