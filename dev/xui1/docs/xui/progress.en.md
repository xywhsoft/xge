# XUI Progress

`xge_xui_progress_t` is a determinate progress display control. It paints the unloaded track, the loaded fill, and optional text inside the widget content rect. Widget background, border, padding, clipping, enabled state, and owner draw remain Widget responsibilities.

## Design

- Progress is a static value indicator, not an input control. It does not receive focus or handle pointer events.
- The control owns only progress semantics: range, value, fill direction, track color, fill color, and optional text.
- Progress text uses two colors. `textColor` is drawn over the whole track, then the same text is clipped to the fill rect and drawn again with `fillTextColor`.
- Text uses a template string. `NULL` means no text. A template with a real `%` is formatted with one `double percent` argument. `%%` is a literal percent sign.
- The rendered text is cached as `sDisplayText` and refreshed only when the template, range, or value changes. There is no render cache because the visual body is only two rectangles plus optional text.
- Widget background and border should be used for the outer frame. `trackColor` and `fillColor` describe the progress content itself.
- Image progress uses reusable `xge_nine_patch_t` objects. `trackPatch` draws the unloaded track and `fillPatch` draws the current fill rect. If a patch is present it replaces the matching color rectangle.
- Simple image progress can choose how the fill image is displayed. `stretch` scales the full fill image into the current progress rect. `reveal` keeps the full image mapping and shows only the current progress portion, so 50% shows the left half in left-to-right mode.

## Nine Patch

The engine-level nine-patch object is shared by XUI and non-XUI rendering:

```c
#define XGE_NINE_PATCH_STRETCH 0
#define XGE_NINE_PATCH_TILE    1

typedef struct xge_nine_patch_t {
    xge_texture pTexture;
    xge_rect_t tSrc;
    float fX1;
    float fY1;
    float fX2;
    float fY2;
    uint32_t iColor;
    int iMode;
    int bEasyMode;
} xge_nine_patch_t;
```

`fX1, fY1, fX2, fY2` are normalized points for the center rectangle. They define the four split lines of the `#` grid. The simple mode is `0, 0, 1, 1`; it renders as one image in stretch mode or repeated full-source tiles in tile mode.

Nine-patch API:

```c
void xgeNinePatchInitSimple(xge_nine_patch patch, xge_texture texture, xge_rect_t src);
void xgeNinePatchInit(xge_nine_patch patch, xge_texture texture, xge_rect_t src, float x1, float y1, float x2, float y2);
void xgeNinePatchSetMode(xge_nine_patch patch, int mode);
void xgeNinePatchSetColor(xge_nine_patch patch, uint32_t color);
void xgeNinePatchDraw(const xge_nine_patch_t* patch, xge_rect_t dst, uint32_t flags);
```

## API

```c
int xgeXuiProgressInit(xge_xui_progress progress, xge_xui_widget widget);
void xgeXuiProgressUnit(xge_xui_progress progress);

void xgeXuiProgressSetRange(xge_xui_progress progress, float min, float max);
void xgeXuiProgressSetValue(xge_xui_progress progress, float value);
float xgeXuiProgressGetValue(xge_xui_progress progress);

void xgeXuiProgressSetText(xge_xui_progress progress, xge_font font, const char* textTemplate);
void xgeXuiProgressSetTextTemplate(xge_xui_progress progress, const char* textTemplate);
void xgeXuiProgressSetTextColor(xge_xui_progress progress, uint32_t color);
void xgeXuiProgressSetFillTextColor(xge_xui_progress progress, uint32_t color);

void xgeXuiProgressSetColors(xge_xui_progress progress, uint32_t track, uint32_t fill);
void xgeXuiProgressSetFillDirection(xge_xui_progress progress, int fillDirection);
void xgeXuiProgressSetTrackPatch(xge_xui_progress progress, const xge_nine_patch_t* patch);
void xgeXuiProgressSetFillPatch(xge_xui_progress progress, const xge_nine_patch_t* patch);
void xgeXuiProgressSetFillPatchMode(xge_xui_progress progress, int mode);

xge_vec2_t xgeXuiProgressMeasureProc(xge_xui_widget widget, void* user);
void xgeXuiProgressPaintProc(xge_xui_widget widget, void* user);
```

Constants:

```c
#define XGE_XUI_PROGRESS_LEFT_TO_RIGHT 0
#define XGE_XUI_PROGRESS_RIGHT_TO_LEFT 1
#define XGE_XUI_PROGRESS_BOTTOM_TO_TOP 2
#define XGE_XUI_PROGRESS_TOP_TO_BOTTOM 3

#define XGE_XUI_PROGRESS_FILL_STRETCH 0
#define XGE_XUI_PROGRESS_FILL_REVEAL 1
```

Template examples:

```c
xgeXuiProgressSetText(progress, font, NULL);          // no text
xgeXuiProgressSetText(progress, font, "%1.0f%%");     // 42%
xgeXuiProgressSetText(progress, font, "Loading");     // static text
xgeXuiProgressSetText(progress, font, "进度：%1.2f"); // formatted percent value
```

The template parser is intentionally lightweight: any single `%` that is not `%%` marks the template as dynamic. Dynamic templates receive one `double percent` argument.

## XSON

```json
{
  "type": "progress",
  "min": 0,
  "max": 100,
  "value": 42,
  "font": "@fonts.body",
  "text": "%1.0f%%",
  "fillDirection": "leftToRight",
  "trackColor": "#D8ECF8",
  "fillColor": "#2E7CD6",
  "textColor": "#243446",
  "fillTextColor": "#FFFFFF",
  "trackPatch": {
    "texture": "@textures.progressTrack",
    "center": [0.375, 0.25, 0.625, 0.75],
    "mode": "stretch"
  },
  "fillPatch": {
    "texture": "@textures.progressFill",
    "mode": "stretch"
  },
  "fillPatchMode": "reveal"
}
```

Fields:

- `min`, `max`, `value`: numeric range and current value.
- `font`: optional font token or font value.
- `text`: optional template string. `NULL` or missing text means no text.
- `fillDirection`: `leftToRight`, `rightToLeft`, `bottomToTop`, or `topToBottom`. The aliases `rtl`, `btt`, and `ttb` are also accepted.
- `trackColor`: unloaded track color. Alpha 0 skips track drawing.
- `fillColor`: loaded fill color. Alpha 0 skips fill drawing.
- `textColor`: text color over the track/unfilled area.
- `fillTextColor`: text color clipped to the filled area.
- `trackPatch`: optional nine-patch object for the unloaded track.
- `fillPatch`: optional nine-patch object for the loaded fill.
- `fillPatchMode`: `stretch` or `reveal`. `reveal` is intended for simple image fills; nine-patch fills still draw into the current fill rect so rounded ends remain correct.

Nine-patch fields:

- `texture`: texture token or texture pointer value.
- `src`: optional `[x, y, w, h]` source rectangle. Missing or zero size means the full texture.
- `center`: optional `[x1, y1, x2, y2]` normalized center rectangle. Missing means simple mode.
- `mode`: `stretch` or `tile`.
- `color`: optional patch tint.

## Examples

- `examples/xui_progress`: C API version.
- `examples/xui_progress_xson`: XSON version.

Both examples use one vertical layout with ten label/progress pairs: blank text, percent text, static text, custom template text, custom colors, borderless transparent outer background, stretched image progress, revealed image progress, stretched nine-patch progress, and tiled nine-patch progress.
