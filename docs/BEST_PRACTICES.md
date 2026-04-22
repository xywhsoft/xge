# XGE 最佳实践

[返回文档中心](README.md)

## 初始化

后端选择必须发生在 `xgeInit` 前：

```c
xgePlatformBackendSet(&backend);
xgeGraphicsBackendSet(&graphics);
xgeInit(&desc);
```

普通应用不需要手动设置后端，使用默认后端即可。

## 资源生命周期

资源对象由调用方持有，XGE 管理内部引用和后端资源。

建议约定：

- 初始化阶段加载常驻资源。
- 场景进入时加载场景资源。
- 场景退出时释放场景资源。
- 频繁使用资源由上层缓存管理。

## 错误处理

所有返回 `int` 的 API 都应检查结果。开发期至少写日志：

```c
if ( xgeTextureLoad(&texture, path) != XGE_OK ) {
	xgeLogWrite(XGE_LOG_WARN, "asset", path);
}
```

不要把失败静默吞掉，否则跨平台调试成本会很高。

## 渲染状态

修改全局状态后及时恢复：

- `xgeBlendSet`
- `xgeDepthTestSet`
- `xgeCameraSet`
- `xgeViewportSet`
- `xgeClipSet`

UI 和 debug overlay 通常最后绘制，并使用普通 2D 状态。

## 输入

游戏控制使用 key/mouse/touch/gamepad API。文本输入使用 text/IME 事件，不要用 keydown 拼中文输入。

触控逻辑用 touch id 跟踪生命周期，不要用数组下标表达同一根手指。

## XUI

XUI 是 retained-mode GUI。保持控件树稳定，改变状态后标记 dirty 或请求刷新。

布局批量修改时使用：

```c
xgeXuiLayoutBatchBegin(&ui);
/* updates */
xgeXuiLayoutBatchEnd(&ui);
```

## 平台文档措辞

未实机验证的平台只能写：

- 已接入脚手架
- 待验证
- 计划支持

不能写成“已支持”或“已通过”。

## 测试策略

开发期优先：

1. 构建测试。
2. 文档检查。
3. 平台冒烟。
4. 人工验证。

平台验证结果写入 `dev/docs/平台后端验证结果.md`。
