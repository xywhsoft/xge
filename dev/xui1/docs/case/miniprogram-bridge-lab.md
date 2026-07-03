# MiniProgram Bridge Lab

`examples/miniprogram_bridge_lab` 用手动宿主方式覆盖小程序 bridge API。它先跑一段 `xgeMiniProgramInit` + bridge-before-init，再跑一段 `xgeMiniProgramInitSimple` + bridge-after-init，并验证 `res://` 资源桥、touch/text、request-frame、audio command 和 frame pump。

## 覆盖 API

- `xgeMiniProgramInit`
- `xgeMiniProgramInitSimple`
- `xgeMiniProgramUnit`
- `xgeMiniProgramSetBridge`
- `xgeMiniProgramFrame`
- `xgeMiniProgramResize`
- `xgeMiniProgramTouch`
- `xgeMiniProgramTouchOne`
- `xgeMiniProgramText`
- `xgeMiniProgramRequestFrame`
- `xgeMiniProgramAudioCommand`

## 构建和运行

```bat
examples\miniprogram_bridge_lab\build.bat
build\xge_miniprogram_bridge_lab.exe --frames 3
```

## 验证点

- 控制台输出 `miniprogram-bridge-lab final-summary`。
- `desc=1/1/1/1` 表示 bridge-before-init、完整描述初始化、backend 切换和 platform caps 检查通过。
- `before=1/1/1/1/1/1/1` 表示 `res://` 资源桥、touch 数组、touchOne、text、resize、requestFrame、audioCommand 都通过。
- `frames=1/<n>` 表示 `xgeMiniProgramFrame` 能驱动 manual runtime frame callback。
- `simple=1/1/1/1/1/1` 表示 `InitSimple`、bridge-after-init、post-init resource provider 注册、requestFrame、frame pump 和 unit 后状态都通过。
- `bridge(load=... free=... request=... audio=...)` 记录宿主回调命中次数，`last=res://bridge-after.txt` 表示第二阶段也确实走到了资源桥。

## 可跳过条件

- 该范例不依赖真实浏览器、小程序容器或 WebGL window，Windows/GCC 本地应可直接运行；通常不需要跳过。
