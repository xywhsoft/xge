# XUI VirtualJoystick

VirtualJoystick is a game-oriented joystick widget for touch screens, mouse
dragging, and keyboard/gamepad channel input. It is intended for mobile movement
controls, quick in-game direction input, and Windows-compatible WASD or arrow
key fallback.

The default visual uses the built-in joystick atlas assets generated under
`res/xui_builtin_atlas.json`.

## Behavior

- pointer down captures the active mouse, touch, or pen pointer
- pointer movement is converted to a normalized vector in `[-1, 1]`
- pointer input has priority over channel input while the pointer is active
- after pointer release, active channels are applied again automatically
- four channels are provided: left, right, up, and down
- channels accept a pressed flag and a `0..1` component value
- `x = right - left`
- `y = down - up`, so up is negative Y in screen coordinates
- vector length is clamped to 1
- dead zone converts small magnitudes to zero
- the state reports vector, magnitude, angle, source, active flag, and pointer id

## Public API

```c
xuiVirtualJoystickGetType
xuiVirtualJoystickCreate
xuiVirtualJoystickSetChange
xuiVirtualJoystickSetValue
xuiVirtualJoystickReset
xuiVirtualJoystickGetState
xuiVirtualJoystickGetX
xuiVirtualJoystickGetY
xuiVirtualJoystickGetMagnitude
xuiVirtualJoystickGetAngle
xuiVirtualJoystickSetChannel
xuiVirtualJoystickGetChannel
xuiVirtualJoystickClearChannels
xuiVirtualJoystickSetMetrics
xuiVirtualJoystickGetMetrics
xuiVirtualJoystickUseBuiltinAtlas
xuiVirtualJoystickGetUseBuiltinAtlas
xuiVirtualJoystickSetSurface
xuiVirtualJoystickSetColors
xuiVirtualJoystickGetChangeCount
xuiVirtualJoystickGetBaseRect
xuiVirtualJoystickGetKnobRect
```

## Descriptor

```c
typedef struct xui_virtual_joystick_desc_t {
    uint32_t iSize;
    xui_virtual_joystick_change_proc onChange;
    void* pChangeUser;
    float fRadius;
    float fKnobSize;
    float fDeadZone;
    int bUseBuiltinAtlas;
    xui_surface pBaseSurface;
    xui_surface pBaseActiveSurface;
    xui_surface pKnobSurface;
    xui_surface pKnobActiveSurface;
    xui_surface pRippleSurface;
    xui_rect_t tBaseSrc;
    xui_rect_t tBaseActiveSrc;
    xui_rect_t tKnobSrc;
    xui_rect_t tKnobActiveSrc;
    xui_rect_t tRippleSrc;
    uint32_t iBaseColor;
    uint32_t iBaseActiveColor;
    uint32_t iKnobColor;
    uint32_t iKnobActiveColor;
    uint32_t iRippleColor;
    uint32_t iFocusColor;
    uint32_t iDisabledColor;
} xui_virtual_joystick_desc_t;
```

Defaults:

```text
radius: 84
knob size: 54
dead zone: 0.08
use built-in atlas: enabled
focusable: true
tab stop: true
overflow: visible
```

## State

```c
typedef struct xui_virtual_joystick_state_t {
    uint32_t iSize;
    float fX;
    float fY;
    float fMagnitude;
    float fAngle;
    int bActive;
    int iSource;
    uint64_t iPointerId;
    int iPointerType;
} xui_virtual_joystick_state_t;
```

State source values:

```text
XUI_VIRTUAL_JOYSTICK_SOURCE_NONE
XUI_VIRTUAL_JOYSTICK_SOURCE_POINTER
XUI_VIRTUAL_JOYSTICK_SOURCE_CHANNEL
XUI_VIRTUAL_JOYSTICK_SOURCE_PROGRAM
```

## Channel Input

Host applications can feed keyboard, gamepad, or platform-specific input into
the four channel API:

```c
xuiVirtualJoystickSetChannel(joy, XUI_VIRTUAL_JOYSTICK_CHANNEL_LEFT,  leftDown,  1.0f, 1);
xuiVirtualJoystickSetChannel(joy, XUI_VIRTUAL_JOYSTICK_CHANNEL_RIGHT, rightDown, 1.0f, 1);
xuiVirtualJoystickSetChannel(joy, XUI_VIRTUAL_JOYSTICK_CHANNEL_UP,    upDown,    1.0f, 1);
xuiVirtualJoystickSetChannel(joy, XUI_VIRTUAL_JOYSTICK_CHANNEL_DOWN,  downDown,  1.0f, 1);
```

Analog input can pass partial values, for example `0.35f`. The control clamps
each component to `0..1` and clamps the final vector length to 1.

## Atlas Parts

`xuiVirtualJoystickSetSurface` can replace individual visuals:

```text
XUI_VIRTUAL_JOYSTICK_PART_BASE
XUI_VIRTUAL_JOYSTICK_PART_BASE_ACTIVE
XUI_VIRTUAL_JOYSTICK_PART_KNOB
XUI_VIRTUAL_JOYSTICK_PART_KNOB_ACTIVE
XUI_VIRTUAL_JOYSTICK_PART_RIPPLE
```

Built-in atlas names:

```text
virtual_joystick_base
virtual_joystick_base_active
virtual_joystick_ripple
virtual_joystick_knob
virtual_joystick_knob_active
```

If atlas drawing fails or built-in atlas usage is disabled without custom
surfaces, the widget falls back to simple circular geometry.

## Style Properties

```text
virtual_joystick.radius
virtual_joystick.knob_size
virtual_joystick.deadzone
virtual_joystick.atlas.enabled
virtual_joystick.base.color
virtual_joystick.base.active_color
virtual_joystick.knob.color
virtual_joystick.knob.active_color
virtual_joystick.ripple.color
virtual_joystick.focus.color
virtual_joystick.disabled.color
```

## Example

```bat
cd /d D:\git\xge\dev\xui2
examples\xui_virtual_joystick\build.bat
build\xui_virtual_joystick.exe
```

The example supports mouse dragging, touch events, and WASD or arrow key channel
input.

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_virtual_joystick_test.bat
examples\xui_virtual_joystick\build.bat
build\xui_virtual_joystick.exe --frames 5
```

The synthetic example run should report `create=1`, `layout=1`, `channel=1`,
`pointer=1`, and `atlas=1`.
