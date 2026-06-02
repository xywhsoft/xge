# XUI Breadcrumb

Breadcrumb is a horizontal navigation trail. It stores a fixed list of nodes, renders separators between nodes, and emits a click callback when a clickable node is pressed.

Reference: [layui-vue Breadcrumb](https://www.layui-vue.com/zh-CN/components/breadcrumb)

## Behavior

- Nodes are copied into the control during creation or `xuiBreadcrumbSetItems`.
- Each node has independent `bClickable` state and an integer `iValue`.
- Only clickable nodes participate in hover, active, and click handling.
- The last node is often non-clickable, but the control does not force that rule.
- The separator can be text such as `/`, `>`, `-`, or a caller-provided surface icon.

## Public API

```c
xuiBreadcrumbGetType
xuiBreadcrumbCreate
xuiBreadcrumbSetClick
xuiBreadcrumbSetItems
xuiBreadcrumbClearItems
xuiBreadcrumbAddItem
xuiBreadcrumbSetItem
xuiBreadcrumbGetItemCount
xuiBreadcrumbGetItemText
xuiBreadcrumbGetItemClickable
xuiBreadcrumbGetItemValue
xuiBreadcrumbSetSeparator
xuiBreadcrumbGetSeparator
xuiBreadcrumbSetSeparatorIcon
xuiBreadcrumbGetSeparatorIcon
xuiBreadcrumbGetSeparatorIconSource
xuiBreadcrumbGetSeparatorIconSize
xuiBreadcrumbSetFont
xuiBreadcrumbGetFont
xuiBreadcrumbSetTextColors
xuiBreadcrumbSetSeparatorColor
xuiBreadcrumbGetSeparatorColor
xuiBreadcrumbSetBackgroundColor
xuiBreadcrumbGetBackgroundColor
xuiBreadcrumbSetMetrics
xuiBreadcrumbGetItemRect
xuiBreadcrumbGetSeparatorRect
xuiBreadcrumbGetHoverIndex
xuiBreadcrumbGetActiveIndex
xuiBreadcrumbGetClickCount
```

The click callback receives the node index and node value:

```c
typedef void (*xui_breadcrumb_click_proc)(
    xui_widget_t* pWidget,
    int iIndex,
    int iValue,
    void* pUser);
```

## Descriptor

```c
typedef struct xui_breadcrumb_item_t {
    const char* sText;
    int bClickable;
    int iValue;
} xui_breadcrumb_item_t;

typedef struct xui_breadcrumb_desc_t {
    uint32_t iSize;
    const xui_breadcrumb_item_t* pItems;
    int iItemCount;
    const char* sSeparator;
    xui_surface pSeparatorIcon;
    xui_rect_t tSeparatorIconSrc;
    xui_font pFont;
    uint32_t iTextColor;
    uint32_t iHoverTextColor;
    uint32_t iActiveTextColor;
    uint32_t iDisabledTextColor;
    uint32_t iSeparatorColor;
    uint32_t iBackgroundColor;
    float fSeparatorIconSize;
    float fGap;
    float fPaddingX;
    float fPaddingY;
} xui_breadcrumb_desc_t;
```

`XUI_BREADCRUMB_MAX_ITEMS` limits one Breadcrumb to 32 nodes.

## Separator Icon

`xuiBreadcrumbSetSeparatorIcon` switches rendering from text separator to icon separator. Pass `NULL` to clear the icon, or call `xuiBreadcrumbSetSeparator` to switch back to text mode.

The icon is tinted with `iSeparatorColor` or `breadcrumb.separator.color`, so a white mask-style icon works well.

## Style Properties

```text
breadcrumb.text.color
breadcrumb.text.hover_color
breadcrumb.text.active_color
breadcrumb.text.disabled_color
breadcrumb.separator.color
breadcrumb.background.color
breadcrumb.gap
breadcrumb.padding_x
breadcrumb.padding_y
breadcrumb.separator.icon_size
font.name
```

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_breadcrumb_test.bat
examples\xui_breadcrumb\build.bat
build\xui_breadcrumb.exe --frames 5
build_dll.bat
```

The example synthetic run should report `layout=1` and `click=1`.
