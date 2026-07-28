# XUI Icon Manager

## Scope

The icon manager is owned by `xui_context`. It provides category and icon
registration, stable IDs, source lifetime management, lazy loading, SVG size
caching, and a single drawing API.

Toolbar, TreeView, ListView, and other controls are not bound to this manager
yet. Their existing icon APIs remain unchanged until the control integration
phase.

## Data Model

Each context stores:

- an ordered category array;
- a category name dictionary;
- a context-wide list used only for deterministic destruction.

Each category stores:

- `arrIcons`: ordered pointer slots;
- `mapIcons`: icon name to `xui_icon_id`;
- category size, fit, and cache policy;
- active icon count and generation.

`xui_icon_id` is the 1-based array slot. Removing an icon sets its slot to
`NULL`; IDs are never reused during the category lifetime. `xuiIconClear()`
also keeps the old slots, so icons added after a clear receive new IDs.

Use `xuiIconCategoryGetIconAt()` to enumerate active icons in registration
order. Use `xuiIconFindById()` for direct stable-ID lookup.

## Category Size

`XUI_ICON_SIZE_FIXED` gives every icon in the category a canonical logical
size. A 16 x 16 toolbar category therefore draws 16 x 16 content centered in
the destination slot.

`XUI_ICON_SIZE_UNRESTRICTED` uses the requested destination size. This is
appropriate for icon pickers, previews, document thumbnails, and other
variable-size uses.

The category fit mode uses the existing `XUI_IMAGE_*` constants. The default
is `XUI_IMAGE_CONTAIN`.

## Sources

The manager supports:

- SVG path strings;
- SVG files;
- SVG memory;
- raster files;
- raster memory;
- direct XUI surfaces;
- named XUI surface resources;
- aliases;
- custom measure, prepare, draw, and destroy callbacks.

File paths and memory blocks are copied by the icon. SVG path styles also
deep-copy dash arrays.

A direct surface is borrowed unless `XUI_ICON_SURFACE_TAKE_OWNERSHIP` is set.
Named resources are resolved by name on every use, so removing or replacing a
resource never leaves a dangling resource pointer in the icon.

Aliases retain their target icon. Alias chains are bounded during prepare,
measure, and draw.

## References

The context registry owns one reference to each registered category. A
category owns one reference to every registered icon. Handles returned by
create, add, find, and enumeration functions are borrowed.

Call `xuiIconCategoryAddRef()` or `xuiIconAddRef()` only when a handle must
survive removal from its registry. Release that strong reference with the
matching `Release()` call.

All icon handles are context-bound and become invalid when the context is
destroyed.

## Loading And Cache

Raster files and memory are loaded lazily once and reused at every draw size.

Full SVG documents are rasterized lazily at the required physical pixel size.
The cache key is `(pixel width, pixel height)`, so DPI changes naturally create
the correct variant. Each icon uses a small LRU cache whose capacity is set by
its category. The default capacity is four variants.

`xuiIconPrepare()` and `xuiIconCategoryPreload()` move loading work out of a
paint-critical path. `xuiIconTouch()` drops source-derived caches after a file
or memory source changes.

SVG document loading is an optional proxy capability:

- `XUI_PROXY_CAP_SVG_SURFACE`
- `surfaceLoadSvgFile`
- `surfaceLoadSvgMemory`

The XGE proxy implements this capability through `xgeSvgTextureLoad()` and
`xgeSvgTextureLoadMemory()`. XUI core does not depend on XGE SVG types.

## Batch Registration

Use `xuiIconCategoryBeginUpdate()` and `xuiIconCategoryEndUpdate()` when adding
many icons. The category generation and context invalidation advance once at
the end of the batch instead of once per icon.

## Example

Build and run:

```bat
examples\xui_icon\build.bat
build\xui_icon.exe
```

The example shows fixed-size SVG paths, unrestricted full SVG documents,
multiple cached SVG sizes, aliases, and a custom icon.
