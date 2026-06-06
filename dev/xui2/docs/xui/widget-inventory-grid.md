# XUI InventoryGrid

InventoryGrid is a cache-rendered slot grid for game inventory, quickbar, equipment, storage, and loot UI. It owns UI state only: slot visuals, selection, hit testing, scrolling, and drag/drop callbacks. The game still owns item rules, stack logic, ownership, persistence, and equipment validation.

## Behavior

- one widget renders all slots; it does not create one child widget per slot
- fixed columns or automatic columns are supported
- slot data is copied into the control, while icon surfaces and animation objects are weak references
- selection modes support none, single, and multi-selection
- pointer selection, right-click context callbacks, keyboard navigation, activation, wheel scrolling, and drag/drop are built in
- drag/drop reports a suggested mode: move, swap, stack, or copy
- drop callbacks decide whether a drop is accepted; the control does not apply game rules by itself
- stack split popup is built in as an interaction helper; it reports the requested split count and leaves item mutation to the game
- rich tooltip is built on the shared XUI tooltip layer and follows the hovered slot
- animation support stores a weak pointer, flags, scale, and tint on each slot;
  an optional render callback can play visible slot animations without making
  InventoryGrid own the animation system

## Public API

```c
xuiInventoryGridGetType
xuiInventoryGridCreate
xuiInventoryGridSetSlotCount
xuiInventoryGridGetSlotCount
xuiInventoryGridSetSlot
xuiInventoryGridGetSlot
xuiInventoryGridClearSlot
xuiInventoryGridClearAll
xuiInventoryGridSetCurrent
xuiInventoryGridGetCurrent
xuiInventoryGridSetSelected
xuiInventoryGridGetSelected
xuiInventoryGridClearSelection
xuiInventoryGridSetLayout
xuiInventoryGridGetLayout
xuiInventoryGridSetMetrics
xuiInventoryGridSetColors
xuiInventoryGridGetColors
xuiInventoryGridGetScrollModel
xuiInventoryGridGetSlotRect
xuiInventoryGridHitTest
xuiInventoryGridEnsureSlotVisible
xuiInventoryGridGetVisibleRange
xuiInventoryGridGetLastPaintRange
xuiInventoryGridGetLastPaintSlotCount
xuiInventoryGridQuerySlots
xuiInventoryGridSortSlots
xuiInventoryGridSetGamepadProfile
xuiInventoryGridGetGamepadProfile
xuiInventoryGridGamepadButton
xuiInventoryGridSetSelectCallback
xuiInventoryGridSetActivateCallback
xuiInventoryGridSetContextCallback
xuiInventoryGridSetDragCallback
xuiInventoryGridSetDropCallback
xuiInventoryGridSetSplitCallback
xuiInventoryGridSetTooltipVisible
xuiInventoryGridGetTooltipVisible
xuiInventoryGridSetTooltipCallback
xuiInventoryGridSetRenderCallback
xuiInventoryGridSetAnimationRenderCallback
xuiInventoryGridOpenSplitPopup
xuiInventoryGridCommitSplitPopup
xuiInventoryGridCloseSplitPopup
xuiInventoryGridIsSplitPopupOpen
xuiInventoryGridGetSplitPopupWidget
xuiInventoryGridGetSplitInputWidget
xuiInventoryGridGetSplitSlot
xuiInventoryGridGetSplitCount
xuiInventoryGridSetSlotAnimation
xuiInventoryGridGetSlotAnimation
xuiInventoryGridClearSlotAnimation
xuiInventoryGridGetTooltipSlot
xuiInventoryGridGetHoverSlot
xuiInventoryGridGetActiveSlot
xuiInventoryGridGetDragSource
xuiInventoryGridGetDropTarget
xuiInventoryGridGetChangeCount
xuiInventoryGridToXValue
xuiInventoryGridExportXSON
xuiInventoryGridSaveXSONFile
```

## Slot Data

```c
typedef struct xui_inventory_slot_t {
    uint32_t iSize;
    int iSlotId;
    int iItemId;
    int iCount;
    int iMaxCount;
    int iSlotType;
    int iItemType;
    uint32_t iFlags;
    uint32_t iQualityColor;
    uint32_t iIconTint;
    xui_surface pIcon;
    xui_rect_t tIconSrc;
    float fCooldownRate;
    float fDurabilityRate;
    char sText[64];
    char sHotkey[16];
    xui_animation_object_t* pAnimation;
    uint32_t iAnimationFlags;
    float fAnimationScale;
    uint32_t iAnimationTint;
} xui_inventory_slot_t;
```

Useful flags:

```text
XUI_INVENTORY_SLOT_EMPTY
XUI_INVENTORY_SLOT_DISABLED
XUI_INVENTORY_SLOT_LOCKED
XUI_INVENTORY_SLOT_SELECTED
XUI_INVENTORY_SLOT_HIGHLIGHT
XUI_INVENTORY_SLOT_NEW
XUI_INVENTORY_SLOT_EQUIPPED
XUI_INVENTORY_SLOT_COOLDOWN
XUI_INVENTORY_SLOT_DURABILITY
XUI_INVENTORY_SLOT_ANIMATION
XUI_INVENTORY_SLOT_COOLDOWN_RADIAL
```

`XUI_INVENTORY_SLOT_COOLDOWN` uses a rectangular bottom-up overlay by default.
Add `XUI_INVENTORY_SLOT_COOLDOWN_RADIAL` to draw the same `fCooldownRate` as a
radial fan overlay. The radial path uses `drawMeshTriangles`; if the active
proxy does not provide mesh drawing, InventoryGrid falls back to the rectangular
overlay.

## Animation Bridge

`xuiInventoryGridSetSlotAnimation(widget, slot, animation, flags, scale, tint)`
stores a weak animation object pointer and per-slot playback metadata.
InventoryGrid does not own, update, or destroy this object.

`xuiInventoryGridSetAnimationRenderCallback(widget, callback, user)` registers
the bridge used during cache rendering. For each visible slot with a non-null
animation pointer, InventoryGrid computes an animation rect from the slot icon
rect and `scale`, then calls the callback between the icon/text layer and the
cooldown, durability, count, lock, selection, and focus overlays. If no callback
is configured, the animation is skipped.

## Stack Split Popup

`xuiInventoryGridOpenSplitPopup(widget, slot, x, y)` opens a small numeric popup for slots whose `iCount > 1`.
The popup clamps the value to `1..iCount-1`, defaults to half of the stack, and uses `xuiNumericInput` plus OK/Cancel actions.

`xuiInventoryGridSetSplitCallback` receives `(widget, slot, count, user)` after OK or `xuiInventoryGridCommitSplitPopup`.
The control does not move items or change `iCount`; game code should apply inventory rules in that callback.

Pass negative `x` or `y` to anchor the popup near the slot. Right-click integration is intentionally left to the app or example through the normal context callback.

## Rich Tooltip

InventoryGrid registers a tooltip resolver on creation. Hovering a non-empty slot opens a custom tooltip in `XUI_LAYER_TOOLTIP`; empty slots do not show a tooltip.

Default tooltip content includes:

- slot title from `sText`, or `Item <id>` when the title is empty
- stack count and max stack when available
- item id, slot id, and hotkey metadata
- slot quality color as the icon frame color

`xuiInventoryGridSetTooltipVisible(widget, visible)` enables or disables this behavior. Disabling it immediately closes an active InventoryGrid tooltip.

`xuiInventoryGridSetTooltipCallback(widget, callback, user)` lets game code replace the body text while keeping the built-in title, icon frame, and metadata layout. The callback receives `(widget, slot, slotData, buffer, capacity, user)`. Return `XUI_OK` and write text into `buffer` to show a custom body. Return a non-zero value to suppress the tooltip for that slot.

`xuiInventoryGridGetTooltipSlot(widget)` returns the slot currently used by the active tooltip resolver, or `-1` when no slot tooltip is active.

## Virtualized Range

InventoryGrid remains one widget even for large inventories. The renderer computes the row and column window covered by the current scroll offset and only paints slots in that visible window.

`xuiInventoryGridGetVisibleRange(widget, &range)` reports the current visible window:

```c
typedef struct xui_inventory_visible_range_t {
    uint32_t iSize;
    int iFirstSlot;
    int iLastSlot;
    int iSlotCount;
    int iFirstRow;
    int iLastRow;
    int iRowCount;
    int iFirstColumn;
    int iLastColumn;
    int iColumnCount;
    int iTotalRows;
    int iTotalColumns;
    int iPaintSlotCount;
} xui_inventory_visible_range_t;
```

`xuiInventoryGridGetLastPaintRange(widget, &range)` returns the range used by the last cache render. `range.iPaintSlotCount` and `xuiInventoryGridGetLastPaintSlotCount(widget)` report the number of slots actually painted after viewport clipping. These values are useful for profiling and for verifying that huge inventories are not rendered as a full slot array each frame.

## Filter And Sort Helpers

InventoryGrid provides helper APIs that build or sort slot index lists. These helpers do not mutate slot data and do not apply game inventory rules.

```c
typedef int (*xui_inventory_filter_proc)(
    xui_widget_t* widget,
    int slot,
    const xui_inventory_slot_t* data,
    void* user);

typedef int (*xui_inventory_compare_proc)(
    xui_widget_t* widget,
    int slotA,
    const xui_inventory_slot_t* dataA,
    int slotB,
    const xui_inventory_slot_t* dataB,
    void* user);
```

`xuiInventoryGridQuerySlots(widget, &query, slots, capacity, &count)` filters the current slot array and optionally sorts the result. `count` receives the full matched count even when `capacity` is smaller than the result.

```c
typedef struct xui_inventory_slot_query_t {
    uint32_t iSize;
    uint32_t iFlags;
    uint32_t iRequiredFlags;
    uint32_t iRejectedFlags;
    int iSortMode;
    const char* sTextContains;
    xui_inventory_filter_proc onFilter;
    void* pFilterUser;
    xui_inventory_compare_proc onCompare;
    void* pCompareUser;
} xui_inventory_slot_query_t;
```

Useful query flags:

```text
XUI_INVENTORY_QUERY_EXCLUDE_EMPTY
XUI_INVENTORY_QUERY_CASE_INSENSITIVE
XUI_INVENTORY_QUERY_SORT_DESCENDING
```

Built-in sort modes:

```text
XUI_INVENTORY_SORT_NONE
XUI_INVENTORY_SORT_SLOT_INDEX
XUI_INVENTORY_SORT_SLOT_ID
XUI_INVENTORY_SORT_ITEM_ID
XUI_INVENTORY_SORT_ITEM_TYPE
XUI_INVENTORY_SORT_SLOT_TYPE
XUI_INVENTORY_SORT_COUNT
XUI_INVENTORY_SORT_QUALITY
XUI_INVENTORY_SORT_TEXT
XUI_INVENTORY_SORT_CUSTOM
```

Use `iRequiredFlags` and `iRejectedFlags` for flag-based queries, such as selected-only, equipped-only, or excluding locked slots. Use `sTextContains` for title search. Use `onFilter` for game-specific item rules.

## Gamepad Navigation Profile

InventoryGrid provides a control-level gamepad profile without requiring a global XUI gamepad event type. A platform layer or game input adapter can translate device input into `xuiInventoryGridGamepadButton(widget, button, pressed, modifiers)`.

The default profile maps direction, accept, cancel, context, page, home, and end actions to stable `XUI_INVENTORY_GAMEPAD_BUTTON_*` constants. Button value `0` is treated as disabled. Passing `NULL` to `xuiInventoryGridSetGamepadProfile` restores the default mapping.

```c
typedef struct xui_inventory_gamepad_profile_t {
    uint32_t iSize;
    uint32_t iFlags;
    int iAcceptButton;
    int iCancelButton;
    int iContextButton;
    int iLeftButton;
    int iRightButton;
    int iUpButton;
    int iDownButton;
    int iPageUpButton;
    int iPageDownButton;
    int iHomeButton;
    int iEndButton;
} xui_inventory_gamepad_profile_t;
```

Useful profile flags:

```text
XUI_INVENTORY_GAMEPAD_WRAP_ROWS
XUI_INVENTORY_GAMEPAD_WRAP_COLUMNS
XUI_INVENTORY_GAMEPAD_SKIP_DISABLED
XUI_INVENTORY_GAMEPAD_SELECT_ON_MOVE
XUI_INVENTORY_GAMEPAD_MULTI_SELECT_MODIFIERS
```

By default, gamepad movement skips disabled or locked slots and updates selection as it moves. `ACCEPT` activates the current slot, `CONTEXT` fires the existing context callback at the current slot center, and `CANCEL` closes the stack split popup when it is open.

`xuiInventoryGridSortSlots(widget, slots, count, mode, flags, compare, user)` sorts an existing slot index array in place.

## XSON Description

InventoryGrid can export a structured XSON description for tools, debug dumps, and editor integration:

```c
xuiInventoryGridToXValue(widget, &value);
xuiInventoryGridExportXSON(widget, buffer, capacity);
xuiInventoryGridSaveXSONFile(widget, "inventory_grid.xson");
```

The exported root uses `kind = "xui.inventorygrid"`, `version = 1`, and includes:

- `layout`: columns, selection mode, slot size, gap, padding, border, radius, wheel step, and drag threshold
- `colors`: control colors encoded as `#RRGGBBAA`
- `state`: current, hover, active, drag/drop, resolved layout, scroll offset, tooltip, and change counters
- `gamepad`: logical gamepad button profile
- `slots`: slot id, item id, count, type, flags, quality color, icon source rect, text, hotkey, cooldown, durability, and animation metadata

Icon surfaces and animation object pointers are weak runtime references, so XSON stores only descriptive metadata such as `hasIcon` and `animation.hasObject`. Loading item ownership, item database state, icon handles, and animation objects remains the responsibility of the game or editor.

## Layout

```c
typedef struct xui_inventory_grid_layout_t {
    uint32_t iSize;
    int iColumns;
    int iSelectionMode;
    float fSlotSize;
    float fSlotGap;
    float fPadding;
    float fIconPadding;
    float fBorderWidth;
    float fRadius;
    float fWheelStep;
    float fDragThreshold;
} xui_inventory_grid_layout_t;
```

`iColumns = 0` enables automatic column count based on the current widget width. Selection modes are:

```text
XUI_INVENTORY_SELECTION_NONE
XUI_INVENTORY_SELECTION_SINGLE
XUI_INVENTORY_SELECTION_MULTI
```

## Style Properties

```text
inventory.background.color
inventory.slot.background.color
inventory.slot.empty_color
inventory.slot.hover_color
inventory.slot.active_color
inventory.slot.selected_color
inventory.slot.disabled_color
inventory.slot.locked_color
inventory.slot.border_color
inventory.slot.quality_color
inventory.slot.focus_color
inventory.text.color
inventory.text.muted_color
inventory.count.color
inventory.hotkey.color
inventory.cooldown.color
inventory.durability.color
inventory.drag.color
inventory.drop.color
inventory.slot.size
inventory.slot.gap
inventory.padding
inventory.icon.padding
inventory.border.width
inventory.radius
inventory.wheel.step
inventory.drag.threshold
inventory.columns
font.name
```

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_inventory_grid_test.bat
examples\xui_inventory_grid\build.bat
build\xui_inventory_grid.exe --frames 5
build_dll.bat
```

The example synthetic run should report `create=1`, `layout=1`, and `dynamic=1`.
