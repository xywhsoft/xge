# Resource Provider Lab

`examples/resource_provider_lab` 演示 XGE V2 的资源 provider 机制，并把旧版 XGE_V1 `xPack` / `ResManage` 的“统一资源索引和读取”思想迁移成 V2 的 `mem://` 与 `res://` provider。

## 覆盖 API

- `xgeResourceProviderAdd`
- `xgeResourceProviderClear`
- `xgeResourceXPackProviderAdd`
- `xgeResourceLoad`
- `xgeResourceLoadMemory`
- `xgeResourceFree`
- `xgeMemoryFree`

同时通过 provider 间接验证：

- `xgeTextureLoad`
- `xgeFontLoad`

## 构建和运行

```bat
examples\resource_provider_lab\build.bat
build\xge_resource_provider_lab.exe --frames 120
```

可用 `--font <path>` 指定字体文件。默认会优先尝试 Windows 系统字体；如果当前平台找不到可用字体，程序会输出 skip 信息并退出。

## 验证点

- 控制台输出 `resource-provider-lab init`。
- `resources=(mem=1 pack=1 memory=1)` 表示 `mem://`、mock xpack `res://` 和 `xgeResourceLoadMemory` 都通过。
- `texture=1 font=1` 表示纹理和字体都能从 provider 加载。
- `memory_free=1` 表示 `xgeMemoryFree` 已释放由 XGE 分配的内存块。
- `mem(load=... hit=... free=...)` 和 `xpack(read=... hit=... free=...)` 用于观察 provider 命中和释放计数。
