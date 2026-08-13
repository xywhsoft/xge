# XUI RichEdit

`xuiRichEdit` is XUI's structured rich-text editor. The document model is
independent of widgets, block layout is backed by xLayout Flow, and text runs
are rendered directly instead of creating a Label for each run.

## Capabilities

- Paragraphs, headings, block quotes, bullet/number/check lists and horizontal
  rules.
- Text and link runs, inline images and XUI widgets, block images and tables.
- Foreground/background color, bold, italic, underline, strikeout, code,
  subscript/superscript metadata and baseline shift.
- Left, center, right and justified paragraph alignment, indentation, line
  height and paragraph spacing.
- UTF-8 grapheme-safe selection and editing, word navigation, visual-line
  Home/End/Up/Down/PageUp/PageDown, scrolling and IME composition.
- Undo/Redo transactions, command dispatch/query state and a reusable Toolbar
  adapter.
- Multi-format system clipboard exchange for UTF-8 text, native RichEdit JSON
  and semantic HTML.
- Built-in Find/Replace UI with case, whole-word, escape, regex, selection,
  previous/next and transactional Replace All modes.
- Standard scroll models and compact scrollbars, Ctrl+wheel/key zoom, unified
  selection/scroll/zoom/object events and an accessibility semantic tree.
- Stable node-relative positions and a logical-to-visual fragment table for
  hit testing and Markdown source mapping.

## Font faces

XUI does not synthesize bold or italic glyphs. Supply actual faces through
`xuiRichEditSetFontSet`:

```c
xui_rich_font_set_t fonts = {0};
fonts.iSize = sizeof(fonts);
fonts.pNormal = normal;
fonts.pBold = bold;
fonts.pItalic = italic;
fonts.pBoldItalic = bold_italic;
fonts.pHeading1 = heading_1;
fonts.pHeading2 = heading_2;
fonts.pHeading3 = heading_3;
xuiRichEditSetFontSet(editor, &fonts);
```

`fFontSize` is an effective pixel size. RichEdit derives and caches a sized
font from the run's selected face, uses it for both shaping and drawing, and
combines it with the editor zoom. `iWeight` and `iSlant` select the supplied
font-set faces; XUI does not synthesize missing bold or italic outlines.

## Document ownership

```c
xui_rich_document document;
xui_rich_node paragraph;
xui_rich_text_style_t style = {0};
xui_rich_edit_desc_t desc = {0};

xuiRichDocumentCreate(&document);
paragraph = xuiRichNodeGetFirstChild(xuiRichDocumentGetRoot(document));
style.iSize = sizeof(style);
style.iTextColor = XUI_COLOR_RGBA(35, 55, 80, 255);
xuiRichDocumentAppendText(document, paragraph, "Structured text", &style);

desc.iSize = sizeof(desc);
desc.pDocument = document;
desc.bOwnDocument = 1;
desc.pFont = normal;
desc.bWordWrap = 1;
xuiRichEditCreate(context, &editor, &desc);
```

When `bOwnDocument` is true, the editor destroys the document. The document
borrows fonts, surfaces and inline widgets. An inline widget must use the same
XUI context and cannot already have another parent; RichEdit attaches it while
the document is mounted.

## Commands

Use `xuiRichEditExecuteCommand` for Undo/Redo, clipboard, inline formatting,
alignment, headings, block quotes and lists. `xuiRichEditQueryCommand` returns
`DISABLED`, `OFF`, `ON` or `MIXED`, which is suitable for toolbar state.
Ctrl+B, Ctrl+I and Ctrl+U call the same command path.

An existing `xuiToolbar` can use the standard editor command set without
sharing private state or transferring ownership:

```c
xuiRichEditSetupToolbar(editor, toolbar, XUI_RICH_TOOLBAR_DEFAULT);
xuiRichEditSyncToolbar(editor, toolbar);

static void toolbar_selected(xui_widget toolbar, int index, int value, void* user)
{
    xui_widget editor = (xui_widget)user;
    (void)value;
    xuiRichEditExecuteToolbarItem(editor, toolbar, index);
}
```

Call `xuiRichEditSyncToolbar` from the RichEdit event callback after selection
or document changes. It also refreshes command tooltips from the active XUI
language table, so runtime language changes and custom language patches are
applied without recreating the toolbar. The helper never retains or destroys
the toolbar.

Objects use explicit APIs:

```c
xuiRichEditApplyLink(editor, "https://example.com");
xuiRichEditInsertImage(editor, &image_desc);
xuiRichEditInsertTable(editor, &table_desc);
xuiRichEditInsertHorizontalRule(editor);
```

Links can dispatch application events without coupling the document model to a
browser or shell action:

```c
static void link_clicked(xui_widget editor, xui_document_node_id_t node,
    const char* url, void* user)
{
    /* Route the URL through the application's navigation policy. */
}

xuiRichEditSetLinkClick(editor, link_clicked, application);
```

Tables expose cell text/style convenience APIs, row/column resize APIs and
`xuiRichTableGetCellDocument`. Every populated cell is an independent rich
document, so it can contain mixed styles, links and inline objects. RichEdit
creates one on-demand child editor when a table cell is activated rather than
one widget per cell.

Images borrow an `xui_surface`; source and alt strings remain metadata for
application resource loading and serialization. A block or inline image may be
created from a surface, a source URI, or both. Persistent documents require a
source URI because GPU surface pointers are process-local.

## Native persistence

The versioned native JSON format is the lossless save format for RichEdit:

```c
char* json = NULL;
size_t json_size = 0;
xui_rich_document loaded = NULL;

if (xuiRichDocumentSerialize(document, 1, &json, &json_size) == XUI_OK) {
    if (xuiRichDocumentDeserialize(json, json_size, &loaded) == XUI_OK) {
        /* loaded owns a fully validated, independent document. */
    }
    xuiRichDocumentFreeSerialized(json);
}
```

Deserialization builds a new document and returns it only after the complete
input has passed type, range, UTF-8, depth, node-count and ID-uniqueness checks.
It never mutates an existing editor document on failure. The native format
preserves node IDs, block and inline structure, text and paragraph styles,
image resource metadata, table geometry and recursively structured cell
documents.

Raw `xui_font`, `xui_surface` and `xui_widget` pointers are never written to a
file. Runs with a direct font pointer, images without a source URI and inline
widgets currently return `XUI_ERROR_UNSUPPORTED`; resource and widget codecs
will provide the explicit reconstruction boundary for those objects.

## Scrolling, zoom and events

`xuiRichEditGetScrollModel`, `xuiRichEditGetHScrollBarWidget` and
`xuiRichEditGetVScrollBarWidget` expose the same scrolling primitives used by
other XUI controls. `xuiRichEditSetZoom` accepts `0.25` through `8.0`; Ctrl+0
restores 100 percent and Ctrl+plus/minus or Ctrl+wheel adjust zoom.

`xuiRichEditSetEvent` reports selection, scroll, zoom and object activation
through `xui_rich_edit_event_t`. Link activation remains a separate callback
because it carries a URL and must pass through application navigation policy.

## Accessibility

RichEdit registers an `xuiWidget` accessibility provider automatically. It
exposes a document root plus paragraphs, headings, quotes, list/check items,
text, links, images, tables and cells, separators and embedded widgets. Nodes
carry stable IDs, world bounds, document ranges, states and structural
metadata. Focus, activation, selection, toggle and scroll-into-view actions
route through the same editor behavior used by mouse and keyboard input.

Platform bridges enumerate with `xuiWidgetGetAccessibleNodeCount` and
`xuiWidgetGetAccessibleNode`, invoke actions with
`xuiWidgetPerformAccessibleAction`, and subscribe once per context through
`xuiSetAccessibilityEventCallback`. `xuiWidgetGetAccessibilityRevision` lets a
bridge discard stale asynchronous snapshots. This backend-neutral provider is
the contract used by a Windows UI Automation or another platform adapter; it
does not place platform accessibility code inside RichEdit.

## HTML and Markdown

`xuiRichDocumentExportHtml` and `xuiRichDocumentImportHtml` exchange semantic
HTML for paragraphs, headings, block quotes, lists and checked tasks, common
inline styles, links, inline/block images, horizontal rules and rich tables.
The importer is a bounded token parser rather than a browser DOM: unknown
containers keep their visible content, while script and style content is
discarded. It does not execute markup or load resources.

`xuiRichDocumentExportMarkdown` and `xuiRichDocumentImportMarkdown` support a
CommonMark/GFM-oriented subset: headings, quotes, nested bullet/number/task
lists, emphasis, strong, strikeout, code spans, links, images, horizontal
rules and pipe tables with rich inline cell content.

HTML and Markdown are interoperability formats and may lose document metadata
that they cannot represent, such as exact font objects or all paragraph paint
properties. Use native JSON for lossless save/load. All returned text buffers
are released with `xuiRichDocumentFreeSerialized`.

## Positions and transactions

Flat offsets use UTF-8 bytes. Block boundaries occupy one byte (`\n`) and
objects use `U+FFFC` (three UTF-8 bytes). Use
`xuiRichDocumentOffsetToPosition` and `xuiRichDocumentPositionToOffset` for
stable node-relative positions.

Wrap related document edits in `xuiRichDocumentBeginTransaction` and
`xuiRichDocumentEndTransaction`; the transaction creates one Undo entry and
one observer notification.

## Markdown boundary

A Markdown editor should map its AST to this document model and reuse the
selection, transaction, command and layout layers. Markdown constrains allowed
node types and adds source serialization; it should not create a second rich
text layout engine.

## Build and verification

From the repository root:

```bat
examples\xui_richedit\build.bat
test_xui\build_rich_document_test.bat
test_xui\build_rich_edit_test.bat
test_xui\build_rich_edit_large_perf_test.bat
```

The example executable is `build\xui_richedit.exe`. It exercises the standard
toolbar, mixed text styles, emoji, links, an inline image, an embedded XUI
widget, a rich table, lists, find/replace, scrolling and zoom.
