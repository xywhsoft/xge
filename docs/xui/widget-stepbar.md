# XUI StepBar

StepBar displays a fixed sequence of steps and highlights the current step. It is a display-only control; applications update progress by calling `xuiStepBarSetCurrent`.

Reference: [layui-vue Step](https://www.layui-vue.com/zh-CN/components/step)

## Styles

```text
XUI_STEP_BAR_STYLE_ARROW     compact segmented arrow bar
XUI_STEP_BAR_STYLE_DOT       horizontal dot step bar
XUI_STEP_BAR_STYLE_VERTICAL  vertical dot step bar
```

All styles use the same step data and current index. Titles are copied into the control, so callers can pass temporary strings during creation.

## Public API

```c
xuiStepBarGetType
xuiStepBarCreate
xuiStepBarSetSteps
xuiStepBarSetTitle
xuiStepBarGetStepCount
xuiStepBarGetTitle
xuiStepBarSetCurrent
xuiStepBarGetCurrent
xuiStepBarSetStyle
xuiStepBarGetStyle
xuiStepBarSetFont
xuiStepBarGetFont
xuiStepBarSetColors
xuiStepBarSetTextColors
xuiStepBarSetMetrics
xuiStepBarGetStepRect
xuiStepBarGetIndicatorRect
xuiStepBarGetChangeCount
```

`iCurrent` is zero-based. For example, `1` means the second step is active.

## Descriptor

```c
typedef struct xui_step_bar_desc_t {
    uint32_t iSize;
    const char* const* ppTitles;
    int iStepCount;
    int iCurrent;
    int iStyle;
    xui_font pFont;
    uint32_t iDoneColor;
    uint32_t iActiveColor;
    uint32_t iPendingColor;
    uint32_t iLineColor;
    uint32_t iTextColor;
    uint32_t iActiveTextColor;
    uint32_t iPendingTextColor;
    uint32_t iBackgroundColor;
    float fBarHeight;
    float fDotRadius;
    float fLineWidth;
} xui_step_bar_desc_t;
```

Defaults:

```text
done color: light green
active color: green
pending color: gray
line color: light gray
bar height: 30
dot radius: 11
line width: 1
```

## Style Properties

```text
stepbar.style
stepbar.done.color
stepbar.active.color
stepbar.pending.color
stepbar.line.color
stepbar.text.color
stepbar.text.active_color
stepbar.text.pending_color
stepbar.background.color
stepbar.bar_height
stepbar.dot_radius
stepbar.line_width
font.name
```

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_step_bar_test.bat
examples\xui_stepbar\build.bat
build\xui_stepbar.exe --frames 3
build_dll.bat
```

The example synthetic run should report `layout=1`, `dynamic=1`, and `current=2`.
