# XUI Carousel

Carousel is a page-container switcher inspired by layui-vue Carousel. It renders one active page at a time, draws bottom indicators, and shows side arrow controllers on hover.

Reference: [layui-vue Carousel](https://www.layui-vue.com/zh-CN/components/carousel)

## Behavior

- each page is an ordinary XUI widget container
- `xuiCarouselGetPageWidget` returns the page container so callers can add custom children
- `xuiCarouselAddPageChild` adds a child directly to a page container
- bottom indicators switch to the clicked page
- side arrows switch to the previous or next page and are visible while the carousel is hovered
- `xuiCarouselNext`, `xuiCarouselPrev`, and `xuiCarouselSetCurrent` switch pages through API
- loop switching is enabled by default, so next from the last page returns to the first page
- autoplay is driven by `xuiUpdate`; transition animation is intentionally deferred

`iCurrent` is zero-based. For example, `0` means the first page.

## Public API

```c
xuiCarouselGetType
xuiCarouselCreate
xuiCarouselSetChange
xuiCarouselSetPageCount
xuiCarouselGetPageCount
xuiCarouselGetPageWidget
xuiCarouselAddPageChild
xuiCarouselSetCurrent
xuiCarouselGetCurrent
xuiCarouselNext
xuiCarouselPrev
xuiCarouselSetAutoPlay
xuiCarouselGetAutoPlay
xuiCarouselSetLoop
xuiCarouselGetLoop
xuiCarouselSetMetrics
xuiCarouselSetColors
xuiCarouselSetFocusColor
xuiCarouselGetHoverIndicator
xuiCarouselGetHoverArrow
xuiCarouselGetChangeCount
```

## Descriptor

```c
typedef struct xui_carousel_desc_t {
    uint32_t iSize;
    xui_font pFont;
    int iPageCount;
    int iCurrent;
    int bLoop;
    int bAutoPlay;
    float fAutoInterval;
    int bShowIndicators;
    int bShowArrowsOnHover;
    float fArrowSize;
    float fIndicatorSize;
    float fIndicatorGap;
    float fIndicatorBottom;
    uint32_t iBackgroundColor;
    uint32_t iArrowColor;
    uint32_t iArrowHoverColor;
    uint32_t iArrowTextColor;
    uint32_t iIndicatorColor;
    uint32_t iIndicatorActiveColor;
    uint32_t iIndicatorHoverColor;
    uint32_t iFocusColor;
} xui_carousel_desc_t;
```

Defaults:

```text
page count: 1
current: 0
loop: enabled
autoplay: disabled
autoplay interval: 3 seconds when enabled without an explicit interval
arrow size: 36
indicator size: 9
indicator gap: 7
indicator bottom offset: 20
background: orange
```

## Style Properties

```text
carousel.background.color
carousel.arrow.color
carousel.arrow.hover_color
carousel.arrow.text_color
carousel.indicator.color
carousel.indicator.active_color
carousel.indicator.hover_color
carousel.focus.color
carousel.arrow.size
carousel.indicator.size
carousel.indicator.gap
carousel.indicator.bottom
carousel.auto.enabled
carousel.auto.interval
carousel.indicator.visible
carousel.arrow.hover_only
font.name
```

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_carousel_test.bat
examples\xui_carousel\build.bat
build\xui_carousel.exe --frames 3
build_dll.bat
```

The example synthetic run should report `create=1`, `layout=1`, and `dynamic=1`.
