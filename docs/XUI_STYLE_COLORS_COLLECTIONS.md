# Collection Color Styles

## Contract

The five controls expose 121 color keys. API/descriptor colors remain the base
values returned by color getters. The effective paint colors use the existing
default, type, named/class/state-class, inline and token resolution machinery.
Clearing an override restores the current API base color. Color presence is
determined by the resolved property's value type, not by a nonzero value or alpha;
both literal 0 and RGB with alpha 0 are valid transparent overrides.

Every color key requests only CACHE | RENDER dirtiness. The private top-down
onPreparePaint hook from core commit 4ca291e synchronizes ScrollFrame colors and
invalidates the viewport before the framework prepares its children. No child
cache callbacks, UpdateBegin/End, drawing-state mutation or layout work is used.
Unchanged style hashes skip this synchronization. Geometry, row indexes, merges,
text layouts and content measurements are not rebuilt for color changes.

Explicit TableView cell/column/row colors retain their existing precedence.
MessageList in this baseline has no per-sender/per-row color fields; handled
onRenderNode callbacks retain complete control over explicit content colors.
Timeline layer and span custom colors are tracked separately from default colors,
including across record movement/removal. SetLayerColor(0) remains transparent;
SetSpanColor(0) retains its existing reset-to-default convention. Default spans
react to theme/API changes even when created before the change. Blank key frames
now paint iBlankKeyFrameColor instead of hardcoded white.

TableView keys apply to true derived widget types. TableGrid and PropertyGrid in
this baseline are composition-based widgets, not TableView subtypes. A TableView
type style reaches their inner TableView; forwarding a container's class/inline
styles belongs to those controls and is outside these changes. ScrollFrame,
ScrollBar and context Menu/Popup have their own independent global style keys;
these changes do not replace their APIs or overwrite custom content renderers.

## Keys

Add the indicated prefix and a dot to every key below.

### ListView: `listview`

| Family | Keys |
| --- | --- |
| Background and border | `background.color`, `border.color`, `focus.color` |
| Rows | `row.color`, `row.hover_color`, `row.selected_color` |
| Text | `text.color`, `text.disabled_color`, `text.selected_color` |
| Scrollbars | `scrollbar.track_color`, `scrollbar.thumb_color`, `scrollbar.hover_color`, `scrollbar.active_color`, `scrollbar.focus_color`, `scrollbar.disabled_color` |

### TableView: `tableview`

| Family | Keys |
| --- | --- |
| Background and header | `background.color`, `header.color`, `header.text_color` |
| Rows and grid | `row.color`, `row.alt_color`, `row.hover_color`, `row.selected_color`, `row.disabled_color`, `grid.color` |
| Text and focus | `text.color`, `text.disabled_color`, `text.selected_color`, `focus.color` |
| Cell chrome | `check.background_color`, `check.mark_color`, `checker.primary_color`, `checker.secondary_color`, `color.fallback_color`, `cell.invalid_color`, `cell.dirty_color`, `cell.editing_color`, `picker.background_color`, `picker.border_color` |
| Scrollbars | `scrollbar.track_color`, `scrollbar.thumb_color`, `scrollbar.hover_color`, `scrollbar.active_color`, `scrollbar.focus_color`, `scrollbar.disabled_color` |

### TreeView: `treeview`

| Family | Keys |
| --- | --- |
| Background and rows | `background.color`, `border.color`, `focus.color`, `row.color`, `row.hover_color`, `row.selected_color` |
| Text | `text.color`, `text.disabled_color`, `text.selected_color` |
| Expanders | `expander.color`, `expander.selected_color`, `expander.background_color`, `expander.disabled_background_color`, `expander.selected_background_color`, `expander.selected_active_background_color`, `expander.active_background_color`, `expander.hover_background_color` |
| Checks and icons | `check.color`, `check.background_color`, `check.mark_color`, `icon.color`, `icon.folder_tab_color`, `icon.folder_body_color`, `icon.file_background_color`, `icon.file_line_color` |
| Scrollbars | `scrollbar.track_color`, `scrollbar.thumb_color`, `scrollbar.hover_color`, `scrollbar.active_color`, `scrollbar.focus_color`, `scrollbar.disabled_color` |

### MessageList: `messagelist`

| Family | Keys |
| --- | --- |
| Background and rows | `background.color`, `border.color`, `row.hover_color`, `row.selected_color` |
| Bubbles | `bubble.self_color`, `bubble.other_color`, `bubble.system_color` |
| Text | `text.self_color`, `text.other_color`, `text.system_color`, `text.meta_color`, `text.selection_color` |
| Avatars and auxiliary blocks | `avatar.self_color`, `avatar.other_color`, `auxiliary.background_color`, `auxiliary.header_color` |

### TimelineView: `timelineview`

| Family | Keys |
| --- | --- |
| Chrome | `background.color`, `corner.color`, `ruler.color`, `layer.color`, `layer.alt_color`, `layer.accent_color`, `grid.color`, `grid.strong_color` |
| Text and states | `text.color`, `text.muted_color`, `selection.color`, `hover.color`, `current.color`, `border.color`, `focus.color`, `disabled.color` |
| Frames and spans | `frame.color`, `frame.key_color`, `frame.blank_key_color`, `span.color`, `span.text_color` |
| Icons | `icon.color`, `icon.hidden_color`, `icon.locked_color` |
| Scrollbars | `scrollbar.track_color`, `scrollbar.thumb_color`, `scrollbar.hover_color`, `scrollbar.active_color`, `scrollbar.focus_color`, `scrollbar.disabled_color` |

Picker background/border defaults follow the effective TableView header/grid
colors with the original 220/255 opacity. An explicit picker color overrides
that derivation, including an alpha-zero color.

## Verification

Run in Windows PowerShell from the worktree root:

```powershell
cmd /c test_xui\build_style_collections_test.bat
foreach ($family in 'list','table','tree','message','timeline') {
    .\test_xui\build_style_collections_test.ps1 $family -Mode regression
}
foreach ($family in 'table','tree','message') {
    .\test_xui\build_style_collections_test.ps1 $family -Mode audit
}
```

The script compiles the checked-out core sources, including xui_accessibility.c,
and never links an older xge.dll. Style tests inspect effective colors for every
key and real draw-proxy output for visible chrome, first-frame style transitions,
transparent values, clear/base restoration, and preserved custom content.
Rendered cascade checks cover default/type/token/class/inline changes and clearing
on the first frame. Every paint check also verifies an unchanged warm frame has
zero extra cache-paint calls and text measurements. They retain index checks.
The audit modes reuse the existing TableView index, TreeView scale (256 KiB stack), and MessageList
display/source/scale suites.

The batch wrapper runs all five style families and is discovered by the existing
run_all_tests build_*_test.bat enumeration.
