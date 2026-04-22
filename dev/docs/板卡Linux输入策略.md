# 板卡 Linux 输入策略

XGE 板卡 Linux 第一版以 EGL/GLES3 为图形基础，输入层不绑定单一窗口系统。

## 路径优先级

1. 如果系统运行 Wayland，优先通过 Wayland seat 获取 keyboard/pointer/touch。
2. 如果系统运行 X11，优先复用 Sokol/X11 路径。
3. 如果是无窗口 kiosk/裸 EGL 环境，使用 evdev 读取 `/dev/input/event*`。
4. gamepad 后续统一走 evdev 或平台可用的游戏手柄 API。

## evdev 基线

evdev 后端只负责把事件转换为 XGE 输入状态：

- `EV_KEY` -> key down/up
- `EV_REL` -> mouse delta / wheel
- `EV_ABS` -> touch / pointer absolute position
- multitouch slot -> `xge_touch_point_t.iId`

## 约束

- 输入设备枚举和权限配置交给部署层处理。
- 第一版不做热插拔复杂策略，只保留重新扫描接口的空间。
- 触摸坐标需要按 framebuffer 尺寸归一化到 XGE 坐标。
- 文本输入/IME 在裸 EGL 环境默认不可用，需上层提供软键盘或平台服务。
