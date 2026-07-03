# Resource API

> Resource API 负责从文件、内存、自定义 provider 和 xpack provider 读取原始资源数据。

[返回 API 索引](README.md) | [资源教程](../guide/resource-intro.md)

---

## 模块定位

Resource 是 XGE 的原始字节加载层。图片、字体、音频等模块可以建立在它之上，但 Resource 本身不解析 PNG、TTF、WAV 等格式，只返回一段内存和释放方式。

路径策略：

- 普通相对路径：以 exe 所在目录作为根路径。
- `res://`：资源协议入口，可映射到压缩包、宿主资源或其他 provider。
- 内存资源：调用者直接传入数据，当前实现会复制一份。
- xpack：作为可选 provider 接入，不进入 XGE 内核强依赖。

## 公共类型

### `xge_resource_t`

资源数据快照。`pData` 是资源数据指针，`iSize` 是字节数，`free` 和 `pUser` 用于释放数据。

### `xge_resource_provider_t`

自定义 provider 描述。包含 scheme、load/free 回调和用户数据。XGE 保存结构体副本，不接管 `pUser`。

### `xge_xpack_provider_t`

xpack 桥接 provider 描述。包含 xpack 实例、包内根路径、read/free 回调和用户数据。

## API 列表

- `xgeResourceProviderAdd`
- `xgeResourceProviderClear`
- `xgeResourceXPackProviderAdd`
- `xgeResourceLoad`
- `xgeResourceLoadMemory`
- `xgeResourceFree`

## Provider 管理

### xgeResourceProviderAdd

注册自定义资源 provider。

**功能：**

你可以把 `res://` 或其他 scheme 映射到压缩包、宿主资源系统、网络缓存或平台资产系统。

**函数原型：**

```c
XGE_API int xgeResourceProviderAdd(const xge_resource_provider_t* pProvider);
```

**参数：**

- `pProvider`：输入参数，不能为 `NULL`。`sScheme` 和 `load` 必须有效，`free` 可以按 provider 策略决定是否提供。

**返回值：**

- 成功返回 `XGE_OK`。
- provider、scheme 或 load 回调无效时返回 `XGE_ERROR_INVALID_ARGUMENT`。
- provider 注册表已满时返回 `XGE_ERROR_RESOURCE_FAILED`。

**资源归属：**

XGE 保存 provider 结构体副本，不复制 `sScheme` 指向的字符串，不接管 `pUser`。调用者必须保证这些数据在 provider 注册期间有效。

**补充说明：**

- 注册和清空 provider 应在初始化阶段或无并发加载时进行。
- provider 的 `load` 返回的数据应能通过同一 provider 的 `free` 释放；如果 `free == NULL`，XGE 会按资源释放路径处理。

**范例代码：**

```c
xge_resource_provider_t provider;
memset(&provider, 0, sizeof(provider));
provider.sScheme = "res";
provider.load = MyLoad;
provider.free = MyFree;
xgeResourceProviderAdd(&provider);
```

**相关 API：**

- `xgeResourceProviderClear`
- `xgeResourceLoad`
- `xgeResourceXPackProviderAdd`

---

### xgeResourceProviderClear

清空资源 provider 注册表。

**功能：**

你可以移除所有自定义 provider 和 xpack provider 注册关系。

**函数原型：**

```c
XGE_API void xgeResourceProviderClear(void);
```

**参数：**

无。

**返回值：**

无。

**资源归属：**

只清空 XGE 内部注册表，不释放调用者 provider 的 `pUser`、xpack 实例或外部上下文。

**补充说明：**

- 不应在资源加载任务运行中调用。

**范例代码：**

```c
xgeResourceProviderClear();
```

**相关 API：**

- `xgeResourceProviderAdd`
- `xgeResourceXPackProviderAdd`
- `xgeResourceFree`

---

### xgeResourceXPackProviderAdd

注册 xpack 资源 provider。

**功能：**

你可以把 xpack 包作为 `res://` 资源来源接入 XGE。

**函数原型：**

```c
XGE_API int xgeResourceXPackProviderAdd(const xge_xpack_provider_t* pProvider);
```

**参数：**

- `pProvider`：输入参数，不能为 `NULL`。`pPack` 和 `read` 必须有效，`free` 可按 xpack 数据所有权策略提供。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法时返回 `XGE_ERROR_INVALID_ARGUMENT`。
- xpack provider 注册表已满或底层 provider 注册失败时返回对应错误码。

**资源归属：**

XGE 保存 `xge_xpack_provider_t` 副本，不接管 `pPack`、`sRoot` 或 `pUser`。调用者必须保证它们在注册期间有效。

**补充说明：**

- 当前实现会把 xpack provider 包装成 scheme 为 `"res"` 的普通 provider。
- 多个 xpack provider 的查找顺序由注册顺序和实现决定。

**范例代码：**

```c
xge_xpack_provider_t xp;
memset(&xp, 0, sizeof(xp));
xp.pPack = pack;
xp.read = XPackRead;
xp.free = XPackFree;
xgeResourceXPackProviderAdd(&xp);
```

**相关 API：**

- `xgeResourceProviderAdd`
- `xgeResourceLoad`
- `xgeResourceProviderClear`

---

## 加载与释放

### xgeResourceLoad

从 URI 加载资源数据。

**功能：**

你可以从普通文件、`res://` provider 或其他自定义 scheme 读取原始字节数据。

**函数原型：**

```c
XGE_API int xgeResourceLoad(const char* sURI, xge_resource_t* pResource);
```

**参数：**

- `sURI`：输入参数，UTF-8 路径或 URI，不能为 `NULL`。
- `pResource`：输出参数，不能为 `NULL`。函数会先清零结构体。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法时返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 文件不存在时返回 `XGE_ERROR_FILE_NOT_FOUND`。
- provider 或文件读取失败时返回对应错误码。

**资源归属：**

成功后 `pResource` 持有数据指针。调用者使用完必须调用 `xgeResourceFree`。

**补充说明：**

- URI 带 scheme 时优先查找 provider。
- 普通路径按 exe 根目录相对路径策略读取。

**范例代码：**

```c
xge_resource_t res;
if (xgeResourceLoad("assets/config.json", &res) == XGE_OK) {
	/* use res.pData / res.iSize */
	xgeResourceFree(&res);
}
```

**相关 API：**

- `xgeResourceFree`
- `xgeResourceProviderAdd`
- `xgeResourceLoadMemory`

---

### xgeResourceLoadMemory

把内存数据包装成 resource。

**功能：**

你可以把调用者已有的一段内存复制为 `xge_resource_t`，交给依赖 Resource API 的解析流程统一处理。

**函数原型：**

```c
XGE_API int xgeResourceLoadMemory(const void* pData, int iSize, xge_resource_t* pResource);
```

**参数：**

- `pData`：输入参数，不能为 `NULL`。
- `iSize`：输入参数，字节数，不能小于 `0`。为 `0` 时返回空资源。
- `pResource`：输出参数，不能为 `NULL`。

**返回值：**

- 成功返回 `XGE_OK`。
- 参数非法时返回 `XGE_ERROR_INVALID_ARGUMENT`。
- 内存分配失败时返回 `XGE_ERROR_OUT_OF_MEMORY`。

**资源归属：**

当前实现会复制 `pData` 到 XGE 分配的内存中。成功后调用者必须调用 `xgeResourceFree` 释放 resource。

**补充说明：**

- `iSize == 0` 时不会分配数据，返回空 resource。

**范例代码：**

```c
xge_resource_t res;
xgeResourceLoadMemory(data, size, &res);
xgeResourceFree(&res);
```

**相关 API：**

- `xgeResourceLoad`
- `xgeResourceFree`
- `xgeImageLoadMemory`

---

### xgeResourceFree

释放 resource 数据。

**功能：**

你可以释放 `xgeResourceLoad` 或 `xgeResourceLoadMemory` 成功返回的数据。

**函数原型：**

```c
XGE_API void xgeResourceFree(xge_resource_t* pResource);
```

**参数：**

- `pResource`：输入/输出参数，可以为 `NULL`。空资源会被忽略。

**返回值：**

无。

**资源归属：**

如果 resource 提供了 `free` 回调，调用该回调释放数据；否则使用 `xrtFree`。释放后清空结构体。

**补充说明：**

- 每次成功 load 后都应配对调用。

**范例代码：**

```c
xgeResourceFree(&res);
```

**相关 API：**

- `xgeResourceLoad`
- `xgeResourceLoadMemory`
- `xgeMemoryFree`

---

## 生命周期与所有权

`xgeResourceLoad` 和 `xgeResourceLoadMemory` 成功后，`xge_resource_t` 持有数据指针和大小。调用者使用完必须调用 `xgeResourceFree`。

Provider 由调用者注册，XGE 不接管 provider 回调和 `pUser` 的所有权。调用 `xgeResourceProviderClear` 只清空注册关系。

## 线程约束

资源加载可以在资源线程执行，但 provider 本身必须满足调用方选择的线程模型。注册和清空 provider 应在初始化阶段或无并发加载时进行。

## 后端差异

桌面相对路径以 exe 路径为根。Android、iOS、Web 和小程序可能没有普通文件系统，应该通过 `res://` provider、内存加载或宿主资源桥接接入。

## 常见错误

| 问题 | 原因 | 处理方式 |
| --- | --- | --- |
| `res://` 加载失败 | 没有注册对应 provider | 启动时调用 `xgeResourceProviderAdd` 或 `xgeResourceXPackProviderAdd`。 |
| resource 数据泄漏 | 忘记调用 `xgeResourceFree` | 每次成功 load 后都配对释放。 |
| 路径在不同工作目录下失效 | 依赖进程 cwd | 使用 exe 根目录相对路径策略。 |

## 相关示例

- `examples/async`
- `examples/texture`
