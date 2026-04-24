# XUI HyperLink Lab

`examples/xui_hyperlink_lab` 用 V2 现有的 `xgeXuiButton` primitive 组合出 hyperlink 语义，专门补齐 V1 `323.XUI HyperLink Element` 的迁移空缺。它覆盖 default visited、custom palette、transient(no visited) 和 disabled 四种 link 状态，并把 mouse 与 keyboard 激活路径都做成可自动退出的自检。

## 覆盖 API

- `xgeXuiButtonInit`
- `xgeXuiButtonSetClick`
- `xgeXuiButtonSetText`
- `xgeXuiButtonSetTextColor`
- `xgeXuiButtonSetColors`
- `xgeXuiButtonGetState`
- `xgeXuiWidgetSetEnabled`
- `xgeXuiWidgetSetCaptureEventUser`
- `xgeXuiSetFocus`
- `xgeXuiDispatchEvent`

## 构建和运行

```bat
examples\xui_hyperlink_lab\build.bat
build\xge_xui_hyperlink_lab.exe --frames 5
```

## 验证点

- 控制台会输出 `xui-hyperlink-lab final-summary ...`
- `default=1 custom=1 transient=1 disabled=1` 表示四种 hyperlink 语义都通过。
- `mouse=1 keyboard=1` 表示鼠标点击和 Enter/Space 激活路径都已覆盖。
- `visited=1` 表示 default/custom link 会进入 visited，transient link 不会进入 visited。
- `open=1/2/1/0` 表示 default 一次、custom 两次、transient 一次、disabled 零次。
- `disabled_state=1` 表示 disabled link 仍保持不可激活状态。

## 人工观察建议

- 第一条 link 点击后移开鼠标，应变成 visited 色。
- 第二条 link 在 hover、press、focus、visited 下应出现明显不同的文字颜色。
- 第三条 link 点击后恢复 normal，不保留 visited 色。
- 第四条 link 应是 disabled 色，且无法被激活。
