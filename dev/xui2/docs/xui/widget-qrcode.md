# XUI QRCode

QRCode renders a string value as a QR code. It is intended for links, short text, IDs, and other compact values that need to be displayed directly inside an XUI interface.

Reference: [layui-vue QRCode](https://www.layui-vue.com/zh-CN/components/qrcode)

## Scope

- encode `Value` dynamically when it changes
- render the encoded module matrix directly in the widget cache
- support foreground and background colors
- support an optional center icon surface
- keep the API small and dependency-free

The current encoder uses QR Byte mode, error correction level M, and automatic version selection from 1 to 10. Version 10 is enough for common short text and medium-length links.

## Public API

```c
xuiQrCodeGetType
xuiQrCodeCreate
xuiQrCodeSetValue
xuiQrCodeGetValue
xuiQrCodeSetColors
xuiQrCodeSetForegroundColor
xuiQrCodeGetForegroundColor
xuiQrCodeSetBackgroundColor
xuiQrCodeGetBackgroundColor
xuiQrCodeSetPadding
xuiQrCodeGetPadding
xuiQrCodeSetIcon
xuiQrCodeGetIcon
xuiQrCodeGetIconSource
xuiQrCodeGetIconSize
xuiQrCodeSetVersionRange
xuiQrCodeGetVersion
xuiQrCodeGetModuleCount
xuiQrCodeGetModule
xuiQrCodeGetChangeCount
```

`xuiQrCodeGetModule` is mainly for tests and diagnostics. Normal UI code should use `xuiQrCodeSetValue`, colors, padding, and icon APIs.

## Descriptor

```c
typedef struct xui_qrcode_desc_t {
    uint32_t iSize;
    const char* sValue;
    xui_surface pIconSurface;
    xui_rect_t tIconSrc;
    uint32_t iForegroundColor;
    uint32_t iBackgroundColor;
    float fPadding;
    float fIconSize;
    int iMinVersion;
    int iMaxVersion;
} xui_qrcode_desc_t;
```

Defaults:

```text
foreground: black
background: white
padding: 8 px
version range: 1..10
icon size: automatic, capped to 32% of QR size
```

## Style Properties

```text
qrcode.foreground_color
qrcode.background_color
qrcode.padding
qrcode.icon_size
```

## Behavior

- `xuiQrCodeSetValue` rebuilds the QR matrix immediately and invalidates layout/cache/render.
- If the string cannot fit into the configured version range, the setter returns `XUI_ERROR_BUFFER_TOO_SMALL`.
- The icon is drawn after the QR modules and gets a small background-colored backing rectangle to protect scan readability.
- The widget keeps a square content measurement by default and centers the rendered QR matrix inside the actual content rect.

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_qrcode_test.bat
examples\xui_qrcode\build.bat
build\xui_qrcode.exe --frames 3
build_dll.bat
```

The example synthetic run should report `dynamic=1`, `layout=1`, and a positive module count.
