# XUI InventoryGrid Control Design

This document defines the planned XUI2-native `InventoryGrid` control. The control is intended for game inventory UI, quick equipment bars, equipment slots, loot grids, storage boxes, and similar slot-based item surfaces.

## Goals

- Add one reusable `xuiInventoryGrid` typed widget.
- Support backpack grids, quick bars, equipment slots, storage containers, and loot grids through configuration.
- Render many slots efficiently without creating one child widget per slot.
- Support atlas icons, item counts, quality frames, cooldown/durability overlays, selection, hover, active, disabled, locked, drag source, and drag target states.
- Provide slot hit testing and slot rectangle queries for tooltips, guide arrows, effects, and game logic integration.
- Keep game item ownership outside the widget.
- Reserve an animation object hook for future item/equipment slot effects without depending on the animation system now.

## Non-Goals

- No game item database.
- No item rule engine.
- No automatic equipment validation beyond calling application callbacks.
- No item split-count dialog in V1.
- No tooltip content editor in V1.
- No game inventory persistence in the control. The post-V1 XSON slice exports
  a UI description only; it does not reload item ownership, icon handles, or
  game database state.
- No dependency on a future animation system in V1.

## Design Principle

`InventoryGrid` is a UI control, not an inventory model. It owns presentation state and interaction state. The game owns item identity, item rules, stack rules, durability rules, and drag/drop confirmation.

The control stores enough slot data to render and interact:

```text
slot id
item id
slot type
item type
icon surface and atlas rect
count and max count
quality color
cooldown rate
durability rate
state flags
hotkey text
custom text
future animation object pointer
```

The control must not directly mutate game inventory semantics. It may update its local slot data when the application explicitly calls `xuiInventoryGridSetSlot`, or after a callback reports that a move/swap/stack operation is accepted.

## Control Model

```text
InventoryGrid widget
  layout state
    slot count
    columns
    slot size
    gap
    padding
    scroll model
  slot data array
    visible slot records
    icon references
    animation references
  interaction state
    hover slot
    active slot
    selected slots
    current keyboard slot
    drag source
    drag preview
    drop target
  rendering cache
    background
    visible slot window
    slot overlays
    drag overlay
```

Slots are not child widgets. A grid with 200 slots, or 10000 slots, should still be one widget with one slot array and one cache strategy. Rendering uses the scroll model to compute the visible row/column window and skips slots outside that window. This keeps memory, event dispatch, and cache render cost predictable for game UI.

## Slot Data

Proposed public slot structure:

```c
typedef struct xui_animation_object_t xui_animation_object_t;

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

`xui_animation_object_t` is only forward-declared in V1. The grid stores the pointer but does not update, draw, or destroy it until the animation system exists.

Potential slot flags:

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
```

Potential hit parts:

```text
XUI_INVENTORY_HIT_NONE
XUI_INVENTORY_HIT_SLOT
XUI_INVENTORY_HIT_ICON
XUI_INVENTORY_HIT_COUNT
XUI_INVENTORY_HIT_HOTKEY
XUI_INVENTORY_HIT_SCROLLBAR
```

## Layout

The layout model is a repeated slot grid.

```text
columns > 0: fixed columns
columns <= 0: auto columns from available width
rows: derived from slot count and columns
slot size: fixed square by default
gap: spacing between slots
padding: outer grid padding
scroll: enabled when content exceeds viewport
```

Quick bar mode is just one row:

```text
columns = slot count
scroll disabled
hotkey labels visible
selection enabled
```

Equipment mode is a sparse/fixed grid:

```text
slot count fixed
slot type per slot
optional empty slot icon
drop callback validates item type
animation object hook often used for highlight/equipment effects
```

Backpack mode is a normal multi-row grid:

```text
columns fixed or auto
scroll enabled
selection and drag enabled
```

## Rendering

Rendering order:

```text
control background
slot background
quality border / slot type frame
item icon
item animation overlay
cooldown overlay
durability bar
count text
hotkey text
custom text / badges
locked or disabled overlay
hover / active / selected / drag target frame
focus frame
drag preview overlay
```

V1 rendering should use existing proxy capabilities only:

- rect fill/stroke
- round rect fill/stroke if configured radius is non-zero
- surface draw for icons
- text draw for count/hotkey

No new proxy capability is required for V1. Cooldown can start as a rectangular overlay or clipped vertical overlay. Radial cooldown uses the existing mesh triangle route when available and falls back to the rectangular overlay otherwise.

Large inventories use a virtual render range:

- layout resolves total rows and columns from slot count, metrics, and viewport
- `xuiInventoryGridGetVisibleRange` reports the currently visible row/column and slot span
- cache render iterates only that visible row/column span
- `xuiInventoryGridGetLastPaintRange` and `xuiInventoryGridGetLastPaintSlotCount` expose the last painted slot count for profiling and tests

## Animation Reservation

The animation object hook is a weak reference:

- `InventoryGrid` does not own the animation object.
- `InventoryGrid` does not destroy it.
- `InventoryGrid` does not call animation update in V1.
- `InventoryGrid` stores per-slot animation scale, tint, and flags.
- V1 rendering calls an application-provided animation render callback when a
  visible slot has an animation object. If no callback is configured, the
  animation is skipped.

The rendering bridge is deliberately narrow:

```text
resolve slot animation object
compute animation rect from slot icon rect
call application animation renderer between icon/text and overlays
respect disabled/locked/selected visual state
do not change slot model ownership
```

This keeps equipment glow, rarity shine, active quickbar pulse, cooldown finish flash, and quest-item effects available later without redesigning slot data.

## Interaction

Pointer behavior:

- hover updates `iHoverSlot`
- click sets current/selected slot
- double-click activates a slot
- right-click opens context callback/menu
- drag starts after threshold
- drop target updates during drag
- release calls move/drop callback

Keyboard behavior:

- arrow keys move current slot through the grid
- Home/End jump to row or grid boundaries
- PageUp/PageDown scroll by visible rows
- Enter/Space activates current slot
- Delete may call remove callback if configured
- number/hotkey matching is application-driven unless a quickbar mode maps keys internally

Gamepad behavior is exposed as a widget-level profile rather than a global XUI event dependency:

- `xuiInventoryGridGamepadButton` accepts stable logical button IDs from a platform or game input adapter
- `xuiInventoryGridSetGamepadProfile` remaps direction, accept, cancel, context, page, home, and end actions
- movement can skip disabled slots, update selection, wrap by row, wrap by column, or preserve multi-select modifiers
- accept reuses the activation callback
- context reuses the context callback and reports the current slot center
- cancel closes the stack split popup when it is open

Selection behavior:

```text
none: no selected slot, only hover/current
single: one selected slot
multi: Ctrl toggles, Shift range selects
```

Drag/drop modes:

```text
XUI_INVENTORY_DROP_MOVE
XUI_INVENTORY_DROP_SWAP
XUI_INVENTORY_DROP_STACK
XUI_INVENTORY_DROP_COPY
```

The control should compute the suggested drop mode from source/target state, then ask application code to confirm.

## Filter And Sort Helpers

InventoryGrid can expose helper APIs for building slot index views:

```text
slot data array -> filter predicate -> optional sort -> slot index list
```

These helpers do not mutate slot data and do not decide game semantics. They are intended for:

- search panels
- filtered equipment lists
- quickbar configuration tools
- paging or external list views
- game-owned inventory reorder commands

Built-in predicates should stay generic: required slot flags, rejected slot flags, empty-slot exclusion, and title substring search. Game-specific matching, such as item class, rarity tiers, equipment legality, or ownership rules, should use a callback.

Built-in sort modes should stay generic: slot index, slot id, item id, item type, slot type, count, quality color, and text. Custom compare callbacks cover game-specific ordering.

## Public API Direction

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
xuiInventoryGridGetSlotRect
xuiInventoryGridHitTest
xuiInventoryGridEnsureSlotVisible
xuiInventoryGridGetVisibleRange
xuiInventoryGridGetLastPaintRange
xuiInventoryGridGetLastPaintSlotCount
xuiInventoryGridQuerySlots
xuiInventoryGridSortSlots

xuiInventoryGridSetSelectCallback
xuiInventoryGridSetActivateCallback
xuiInventoryGridSetContextCallback
xuiInventoryGridSetDragCallback
xuiInventoryGridSetDropCallback
xuiInventoryGridSetRenderCallback
xuiInventoryGridSetAnimationRenderCallback

xuiInventoryGridSetMetrics
xuiInventoryGridSetColors
xuiInventoryGridGetScrollModel

xuiInventoryGridSetSlotAnimation
xuiInventoryGridGetSlotAnimation
xuiInventoryGridClearSlotAnimation
xuiInventoryGridToXValue
xuiInventoryGridExportXSON
xuiInventoryGridSaveXSONFile
```

Animation APIs are weak-reference setters plus a render callback bridge. They
must not require the animation system implementation.

## XSON Description

The XSON description is a tool/debug/export surface, not the authoritative game inventory save format. It serializes:

- layout metrics and selection mode
- control colors as `#RRGGBBAA`
- runtime UI state such as current slot, drag/drop state, scroll offset, and change count
- gamepad profile
- slot fields used by the renderer and interactions
- animation reservation metadata: flags, scale, tint, and whether a runtime object pointer was present

It intentionally does not serialize raw icon or animation pointers. A game/editor can use the slot ids, item ids, types, text, hotkeys, and flags to map back to its own assets and rules.

## Callback Direction

```c
typedef void (*xui_inventory_select_proc)(xui_widget_t* pWidget, int iSlot, void* pUser);
typedef void (*xui_inventory_activate_proc)(xui_widget_t* pWidget, int iSlot, int iButton, void* pUser);
typedef int (*xui_inventory_drag_proc)(xui_widget_t* pWidget, int iSlot, void* pUser);
typedef int (*xui_inventory_drop_proc)(xui_widget_t* pWidget, int iFromSlot, int iToSlot, int iDropMode, void* pUser);
typedef void (*xui_inventory_context_proc)(xui_widget_t* pWidget, int iSlot, float fX, float fY, void* pUser);
typedef int (*xui_inventory_slot_render_proc)(xui_widget_t* pWidget, int iSlot, const xui_inventory_slot_t* pSlot, xui_draw_context_t* pDraw, xui_rect_t tRect, uint32_t iState, void* pUser);
```

Custom rendering should be an extension point, not the default path. The default slot renderer must be good enough for common game UI.

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
inventory.count.color
inventory.hotkey.color
inventory.text.color
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
font.name
```

Per-item quality color remains part of slot data. Style defines fallback colors and control-level visual defaults.

## Example Plan

The example should show:

- left backpack grid with mixed items, counts, qualities, empty slots, locked slots, and scrolling
- bottom quickbar with hotkeys and current selection
- right equipment grid with slot-type labels and future animation placeholders
- drag/drop between slots
- rejected drop feedback
- status text showing last select/activate/drop callback

## Test Plan

Tests should cover:

- create/destroy
- slot set/get/clear
- layout and slot rects
- hit testing
- scroll model bounds
- selection/current slot movement
- drag/drop callback acceptance and rejection
- cache invalidation on slot update
- icon source rect handling
- animation pointer storage without ownership
- animation render callback rect and metadata
- large slot count visible range and cache render window

## Deferred

- Item filter/sort.
- Gamepad navigation profile.
- XSON load/apply from description.
