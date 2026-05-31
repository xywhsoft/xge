# XUI ScrollView

ScrollView is the user-facing scrollable content widget. It owns a ScrollFrame and a content widget. Application widgets must be added to the content widget.

## Goals

- preserve XUI1 ScrollView behavior while using XUI2 widget clipping and cache rendering
- keep content coordinates stable; child widgets do not subtract scroll offset manually
- expose the underlying frame and model for advanced use
- provide full and compact scrollbar modes through the shared ScrollBar implementation
- keep XSON deferred

## Structure

```text
ScrollView
  ScrollFrame
    viewport widget (overflow clip)
      content widget (positioned at -offsetX, -offsetY)
        application children
    horizontal ScrollBar
    vertical ScrollBar
    corner
```

Content children are attached like this:

```c
xui_widget content = xuiScrollViewGetContentWidget(view);
xuiWidgetAddChild(content, child);
```

The content widget rect is arranged to:

```text
x = -offsetX
y = -offsetY
w = max(contentWidth, viewportWidth)
h = max(contentHeight, viewportHeight)
```

Because content is a child of the clipped viewport, rendering and hit testing outside the viewport are naturally blocked by the parent chain.

## Behavior

ScrollView forwards most scroll behavior to ScrollFrame:

- automatic, always, and hidden scrollbar policies
- full and compact scrollbar modes
- wheel axis selection
- wheel step
- optional content drag
- scroll offset and ensure-rect-visible

`xuiScrollViewEnsureChildVisible` converts a descendant's current world rect back into content coordinates and then calls the model ensure-rect-visible path.

The ScrollView change callback fires for user-driven frame changes. Programmatic setters update the model and content position but do not notify, matching the existing XUI2 control pattern.

## Public API

```c
xuiScrollViewGetType
xuiScrollViewCreate
xuiScrollViewSetChange
xuiScrollViewGetFrameWidget
xuiScrollViewGetContentWidget
xuiScrollViewGetViewportWidget
xuiScrollViewGetModel
xuiScrollViewLayout
xuiScrollViewSetContentSize
xuiScrollViewGetContentSize
xuiScrollViewSetOffset
xuiScrollViewScrollBy
xuiScrollViewGetOffset
xuiScrollViewEnsureRectVisible
xuiScrollViewEnsureChildVisible
xuiScrollViewSetScrollbarPolicy
xuiScrollViewSetScrollbarMode
xuiScrollViewGetScrollbarMode
xuiScrollViewSetWheelAxis
xuiScrollViewGetWheelAxis
xuiScrollViewSetWheelStep
xuiScrollViewSetContentDragEnabled
xuiScrollViewIsContentDragEnabled
xuiScrollViewSetMetrics
xuiScrollViewSetColors
xuiScrollViewGetViewportRect
xuiScrollViewGetChangeCount
```

## Example

```c
xui_scroll_view_desc_t desc;
memset(&desc, 0, sizeof(desc));
desc.iSize = sizeof(desc);
desc.fContentWidth = 640.0f;
desc.fContentHeight = 480.0f;
desc.iPolicyX = XUI_SCROLLBAR_POLICY_AUTO;
desc.iPolicyY = XUI_SCROLLBAR_POLICY_AUTO;
desc.iScrollbarMode = XUI_SCROLLBAR_MODE_COMPACT;

xuiScrollViewCreate(context, &view, &desc);
xuiWidgetSetRect(view, (xui_rect_t){20.0f, 20.0f, 240.0f, 180.0f});
xuiWidgetAddChild(parent, view);

xuiWidgetAddChild(xuiScrollViewGetContentWidget(view), child);
```

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_scroll_view_test.bat
examples\xui_scrollview\build.bat
build\xui_scrollview.exe --frames 360
```

The example summary should include `create=1`, `layout=1`, `model=1`, `ensure=1`, and `input=1`.
