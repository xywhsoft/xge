# XUI InventoryGrid Control SPEC

This SPEC tracks the XUI2-native `InventoryGrid` control. Status markers:

- `[ ]` not started
- `[~]` in progress
- `[X]` completed

## Scope

V1 delivers one `xuiInventoryGrid` widget for game inventory and quickbar UI:

- backpack grid
- quick equipment bar
- equipment slots
- storage or loot grid

The control owns UI state only. The game owns item rules and inventory semantics.

## Maintenance Rules

- Keep this SPEC updated whenever implementation state changes.
- Do not mark a task `[X]` until code, test, example, and documentation evidence exists where applicable.
- Use `xui` public names only; `xui2` is a folder/workstream name.
- Treat XSON as a UI description surface; do not move game inventory
  persistence into the control.
- Do not add proxy APIs silently.
- Preserve cache-first rendering.
- Examples must support `--frames N` and `--seconds N`.
- Do not make one child widget per slot.
- Animation object support is a V1 data/API reservation only unless the animation system exists.

## Phase 0: Design And Planning

- [X] Create InventoryGrid design document: `docs\inventory-grid-control-design.md`.
- [X] Create tracked InventoryGrid SPEC: `docs\inventory-grid-control-spec.md`.
- [X] Add InventoryGrid documents to `docs\README.md`.
- [X] Create widget user document: `docs\xui\widget-inventory-grid.md`.

Completion criteria:

- Design records the model boundary between UI slots and game inventory logic.
- SPEC contains implementation phases and completion gates.
- Docs index links both planning documents.

## Phase 1: Public API And Skeleton

- [X] Add public constants to `xui.h`.
- [X] Add `xui_inventory_slot_t`.
- [X] Add `xui_inventory_grid_desc_t`.
- [X] Add callback typedefs for select, activate, context, drag, drop, and custom render.
- [X] Add weak `xui_animation_object_t` forward declaration.
- [X] Add `xuiInventoryGridGetType`.
- [X] Add `xuiInventoryGridCreate`.
- [X] Add typed widget registration in `src\xui_inventory_grid.c`.
- [X] Add source file to `xui_sources.bat`.
- [X] Add minimal create/destroy/render unit test.

Completion criteria:

- `test_xui\build_inventory_grid_test.bat` builds.
- Empty grid can be created, sized, rendered, and destroyed.
- No new proxy capability is required.

## Phase 2: Slot Model

- [X] Store slot array with configurable slot count.
- [X] Implement `xuiInventoryGridSetSlotCount`.
- [X] Implement `xuiInventoryGridGetSlotCount`.
- [X] Implement `xuiInventoryGridSetSlot`.
- [X] Implement `xuiInventoryGridGetSlot`.
- [X] Implement `xuiInventoryGridClearSlot`.
- [X] Implement `xuiInventoryGridClearAll`.
- [X] Copy text and hotkey strings safely.
- [X] Preserve icon surface reference without ownership.
- [X] Preserve animation object pointer without ownership.
- [X] Add slot model tests.

Completion criteria:

- Slot data round-trips through public API.
- Clearing a slot does not destroy external surfaces or animation objects.
- Updating a slot invalidates render cache.

## Phase 3: Layout And Hit Testing

- [X] Add columns, slot size, gap, padding, and auto-column layout.
- [X] Compute row count and content size.
- [X] Integrate `xui_scroll_model_t`.
- [X] Implement `xuiInventoryGridGetSlotRect`.
- [X] Implement `xuiInventoryGridHitTest`.
- [X] Implement `xuiInventoryGridEnsureSlotVisible`.
- [X] Add visible slot range calculation.
- [X] Add tests for fixed columns, auto columns, padding, gap, and scroll offsets.

Completion criteria:

- Slot rects are pixel-snapped and stable.
- Hit testing maps pointer coordinates to expected slot indices.
- Scrolling keeps content bounds clamped.

## Phase 4: Default Rendering

- [X] Render control background.
- [X] Render slot backgrounds.
- [X] Render empty, disabled, locked, hover, active, selected, and focus states.
- [X] Render quality border.
- [X] Render atlas icon from `pIcon` and `tIconSrc`.
- [X] Render count text.
- [X] Render hotkey text.
- [X] Render custom slot text or badge.
- [X] Render durability bar.
- [X] Render rectangular cooldown overlay.
- [X] Render drag source and drop target frames.
- [X] Reserve animation draw layer without requiring animation playback.
- [X] Add rendering tests with test proxy counters.

Completion criteria:

- Backpack, quickbar, and equipment visuals can be represented by the same renderer.
- Slot updates only dirty the required control cache.
- Rectangular cooldown remains the default path; optional radial cooldown is
  tracked separately below and uses the existing mesh proxy route.

## Phase 5: Selection And Keyboard Navigation

- [X] Add current slot state.
- [X] Add single selection mode.
- [X] Add optional multi-selection mode.
- [X] Implement pointer click selection.
- [X] Implement keyboard arrow navigation.
- [X] Implement Home/End/PageUp/PageDown behavior.
- [X] Implement Enter/Space activation.
- [X] Implement selection callback.
- [X] Implement activation callback.
- [X] Add tests for navigation and selection.

Completion criteria:

- Keyboard and pointer selection agree on current slot.
- Selection remains valid when slot count changes.
- Activation callback reports the expected slot and button.

## Phase 6: Context Menu And Tooltip Hooks

- [X] Add context callback.
- [X] Add right-click slot hit behavior.
- [X] Keep selected slot visible while context menu is open.
- [X] Add optional tooltip request callback or slot rect support for external tooltip.
- [X] Add tests for context slot hit.

Completion criteria:

- Applications can open their own menu using the reported slot and coordinates.
- The control does not hardcode game item menu titles.

## Phase 7: Drag And Drop

- [X] Add drag threshold handling.
- [X] Add drag callback to allow or deny drag start.
- [X] Add drag source state.
- [X] Add drag preview rendering.
- [X] Add drop target hit state.
- [X] Compute suggested drop mode: move, swap, stack, copy.
- [X] Add drop callback.
- [X] Handle rejected drops without mutating slot data.
- [X] Add tests for accepted and rejected drops.

Completion criteria:

- Drag/drop can be connected to game inventory logic.
- The control does not apply game rules without callback approval.
- Failed drops leave slot data unchanged.

## Phase 8: Animation Reservation

- [X] Add `xuiInventoryGridSetSlotAnimation`.
- [X] Add `xuiInventoryGridGetSlotAnimation`.
- [X] Add `xuiInventoryGridClearSlotAnimation`.
- [X] Store animation flags, scale, and tint.
- [X] Ensure animation pointer is not destroyed by grid.
- [X] Add tests for animation pointer storage and clearing.

Completion criteria:

- Animation metadata can be assigned to equipment slots now.
- V1 remains buildable without an animation object implementation.

## Phase 9: Style Properties

- [X] Register `inventory.*` color properties.
- [X] Register `inventory.*` metric properties.
- [X] Resolve fallback style values during render and layout.
- [X] Mark correct dirty flags for style changes.
- [X] Add tests for style-driven colors and metrics.

Completion criteria:

- Runtime style changes invalidate layout/render correctly.
- Per-slot quality color still overrides fallback style color.

## Phase 10: Example And Documentation

- [X] Add `examples\xui_inventory_grid\main.c`.
- [X] Add `examples\xui_inventory_grid\build.bat`.
- [X] Example supports `--frames N`.
- [X] Example supports `--seconds N`.
- [X] Example shows backpack grid.
- [X] Example shows quickbar.
- [X] Example shows equipment slots with animation placeholders.
- [X] Example exercises selection, activation, and drag/drop callbacks.
- [X] Add `docs\xui\widget-inventory-grid.md`.
- [X] Update `docs\work.md` with implementation summary and verification commands.

Completion criteria:

- `examples\xui_inventory_grid\build.bat` succeeds.
- `build\xui_inventory_grid.exe --frames 5` exits cleanly.
- Running without duration stays alive.

## Phase 11: Verification Gate

- [X] Run `test_xui\build_inventory_grid_test.bat`.
- [X] Run `examples\xui_inventory_grid\build.bat`.
- [X] Run `build\xui_inventory_grid.exe --frames 5`.
- [X] Run `build_dll.bat`.
- [X] Run `git diff --check`.
- [X] Record final command output summary in `docs\work.md`.

Completion criteria:

- All V1 commands pass.
- No whitespace errors are reported.
- The SPEC accurately reflects completed and deferred work.

## Post-V1 Follow-Up

- [X] Radial cooldown rendering.
- [X] Stack split popup.
- [X] Rich tooltip widget integration.
- [X] Huge inventory virtualization.
- [X] Filter and sort helpers.
- [X] Gamepad navigation profile.
- [ ] Animation object playback.
- [X] XSON description.
  - [X] Add `xuiInventoryGridToXValue`.
  - [X] Add `xuiInventoryGridExportXSON`.
  - [X] Add `xuiInventoryGridSaveXSONFile`.
  - [X] Export layout, colors, runtime UI state, gamepad profile, slots, and animation metadata.
  - [X] Verify exported XSON through unit test and parse saved file back with XRT.
