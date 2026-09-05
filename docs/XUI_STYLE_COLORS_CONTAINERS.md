# Container stylesheet colors

Color properties use the existing default, type, named, class, state-class,
inline and token cascade. They invalidate style/cache/render only. A color of
zero is a present transparent override, not a request for the default. Public
color setters/getters retain API/base colors; clearing stylesheet overrides
restores the latest base colors.

## ScrollFrame and ScrollView

- `scrollframe.background.color`, `scrollframe.corner.color`, `scrollframe.grip.color`
- `scrollview.background.color`, `scrollview.corner.color`, `scrollview.grip.color`

ScrollView colors override the corresponding colors of its owned ScrollFrame.
The frame resolves them while painting, without changing its base palette.
The ScrollView properties are inherited by its backing frame, so normal style
invalidation and cache preparation handle runtime changes without invoking
child rendering callbacks or clearing their dirty flags manually.

The baseline core resolves inherited styles for global stylesheet changes but
not for local class/inline changes. The regression test intentionally checks
those local changes without an explicit `xuiStyleRefresh`: integration needs
the shared style resolver to propagate inherited-property changes to descendants.

Scrollbars delegate to the actual ScrollBar widgets returned by
`xuiScrollFrameGetHScrollBarWidget` / `xuiScrollFrameGetVScrollBarWidget` (or
the ScrollView accessors). Use `scrollbar.track.color`, `scrollbar.thumb.color`,
`scrollbar.thumb.hover_color`, `scrollbar.thumb.active_color`,
`scrollbar.focus.color`, `scrollbar.disabled.color`, `scrollbar.button.color`
and `scrollbar.button.icon_color` on the ScrollBar type or these child widgets.
No parent aliases rewrite child inline styles or base colors.

## Verification

From the detached workspace in Windows PowerShell:

```powershell
cmd /c test_xui\build_style_containers_test.bat
```

The standalone build uses current project sources, including
`src/xui_accessibility.c`, rather than a prebuilt XGE DLL. Tests observe colors
sent to the drawing proxy after a single cache preparation pass, including
cached descendants, token changes, transparent overrides and clearing styles.
