# XUI and xLayout integration

XUI uses xLayout as its only general-purpose geometry solver. XUI owns widget
state, rendering, input, scrolling, and control behavior. xLayout owns
measurement and arrangement of widget and virtual layout trees.

## Ownership

- One `xui_context` owns one `xlayout_context_t`.
- Every live `xui_widget` owns one `xlayout_node_t` in that context.
- Widget insertion, removal, ordering, visibility, and destruction are mirrored
  to the xLayout tree by XUI.
- Applications configure XUI layout properties. They do not need access to the
  widget's xLayout node.
- A control may own private virtual xLayout nodes for non-widget structures such
  as DockPanel pane trees. Those nodes stay in the same context.

## Standard lifecycle

The complete control layout protocol is:

1. `onLayoutPrepare` updates declarative constraints before synchronization.
2. `onContentMeasure` reports intrinsic content size under the supplied
   constraint.
3. xLayout measures and arranges the complete subtree.
4. `onLayoutChildren` may assign direct child slots for a specialized container.
5. XUI copies results to widgets from parent to child.
6. `onLayoutComplete` derives post-layout state after all descendants are valid.

There are no separate XUI Measure or Arrange compatibility callbacks.

### Leaf controls

A leaf reports content size with `onContentMeasure`. The callback measures only
the control's intrinsic content. xLayout applies padding, explicit sizes,
minimums, maximums, margins, growth, shrinkage, and parent constraints.

### Declarative containers

Most containers only select an XUI layout type and configure child properties.
Row, Column, Flow, Dock, Table, Grid, Overlay, and Manual map directly to xLayout
formatting contexts.

Dynamic containers use `onLayoutPrepare` to update visibility, preferred sizes,
tracks, row or column placement, and similar input state. Prepare must not write
final child rectangles.

### Specialized containers

Controls whose slots depend on private state use `onLayoutChildren` and
`xuiLayoutArrangeChild`. The callback can arrange direct children only and can
be called only while xLayout is arranging that parent. Descendant recursion and
callback ordering remain owned by the common solver.

Examples include SplitLayout panes, ScrollFrame viewport and bars, Tabs pages,
Window chrome, and DockPanel hosts.

### Post-layout state

`onLayoutComplete` runs after arranged descendant geometry has been copied back
to XUI. It may compute text bounds, hit regions, virtual rows, scrollbar model
state, or other non-widget geometry. It must not run another child layout.

Complete may update a constraint discovered from final geometry, such as a
scrollable content extent. XUI preserves that invalidation and runs another
root pass in the same `xuiLayout` call.

## Stabilization and diagnostics

`xuiLayout` runs until both the main and overlay trees are stable. A normal
layout uses one pass; a call with no layout changes uses zero passes. Cyclic
callbacks are stopped after `XUI_LAYOUT_MAX_PASSES` and return
`XUI_ERROR_LAYOUT_UNSTABLE`.

Use `xuiGetLayoutStats` after a layout call:

```c
xui_layout_stats_t stats;
memset(&stats, 0, sizeof(stats));
stats.iSize = sizeof(stats);
xuiGetLayoutStats(context, &stats);
printf("passes=%d stable=%d\n", stats.iPassCount, stats.bStabilized);
```

Repeated pass counts above one should be intentional. A layout that always
reaches the pass limit has a callback or setter that invalidates unchanged
state.

## Invalidation rules

- Intrinsic content changes invalidate measurement.
- Position-only or viewport changes invalidate arrangement.
- Tree, visibility, track, and layout-style changes are synchronized before the
  next solve.
- Setters must return without invalidation when the normalized value is
  unchanged.
- Scroll offsets and virtualized content coordinates are control behavior, but
  viewport, scrollbar, and real content-widget slots still use xLayout.
- Stable passes reuse xLayout measurement caches and workspace allocations.

## Layout mapping

| XUI layout | xLayout format |
| --- | --- |
| Row | horizontal Stack |
| Column | vertical Stack |
| Flow | Flow |
| Dock | Dock |
| Table and Grid | Track |
| Overlay and Manual | Layer |

Manual is reserved for genuinely free-positioned content. It is not an escape
hatch for relationships expressible with Stack, Track, Dock, Flow, or Layer.

Track supports cross-axis sizing. A cross column derives width from available
row height; a cross row derives height from available column width. This keeps
square slots declarative without using a previous-frame rectangle.

## Complex control boundary

- Tabs, Window, Accordion, PropertyGrid, NumericInput, ComboBox, TagInput, and
  ScrollFrame submit their real child slots through xLayout.
- ListView, TreeView, TableView, TimelineView, InventoryGrid, and Terminal keep
  only virtual item geometry inside the control.
- ScrollView positions its real content widget through the viewport's
  `onLayoutChildren`; scrolling does not recursively arrange the content tree.
- DockPanel uses private virtual xLayout nodes for pane splits and submits host
  widget rectangles through the standard specialized-container protocol.
- Popup panels and in-place editors are explicit overlay placement boundaries.
  When immediate placement is required they use the common xLayout-backed
  `xuiWidgetArrange`, not a control-specific solver.

## Verification

```bat
test_xui\build_layout_test.bat
test_xui\build_layout_callback_test.bat
test_xui\build_scroll_frame_test.bat
test_xui\build_scroll_view_test.bat
examples\xui_layout\build.bat
```

The callback test covers specialized child placement, Complete ordering,
two-pass stabilization, zero-pass cache reuse, unstable-cycle detection, and
recovery.
