# 资源加载入门

> 状态：中文初稿已生成，待审阅。

Resource API 是 XGE 的原始字节加载层。它只负责把文件、内存、`res://` 或压缩包里的数据读出来，不负责解析图片、字体或音频格式。

## 路径规则

XGE 的普通相对路径以 exe 所在目录作为根目录，而不是依赖进程当前工作目录。

```text
assets/player.png
assets/ui/font.ttf
```

`res://` 是资源协议入口，适合映射到压缩包、宿主资源、小程序资源或自定义 VFS。

```text
res://ui/button.png
res://level/001.json
```

## 直接加载原始资源

```c
xge_resource_t res;
memset(&res, 0, sizeof(res));

if ( xgeResourceLoad("assets/config.json", &res) == XGE_OK ) {
	/* res.pData / res.iSize */
	xgeResourceFree(&res);
}
```

每次 `xgeResourceLoad` 成功后，都必须调用 `xgeResourceFree`。

## 从内存包装资源

```c
static const char data[] = "{ \"name\": \"demo\" }";
xge_resource_t res;

xgeResourceLoadMemory(data, (int)sizeof(data) - 1, &res);
/* use res */
xgeResourceFree(&res);
```

内存资源适合测试、嵌入式默认配置和宿主已经拿到数据的场景。

## 自定义 provider

自定义 provider 让 `res://` 或其他 scheme 接到自己的资源系统。

```c
static int LoadResource(const char* uri, void** data, int* size, void* user)
{
	(void)uri;
	(void)data;
	(void)size;
	(void)user;
	return XGE_ERROR_UNSUPPORTED;
}

xge_resource_provider_t provider;
memset(&provider, 0, sizeof(provider));
provider.sScheme = "res";
provider.load = LoadResource;
xgeResourceProviderAdd(&provider);
```

Provider 返回的数据必须能通过对应 `free` 回调释放。

## xpack provider

xpack 是可选 VFS 来源，不进入 XGE 内核强依赖。XGE 通过回调接入它。

```c
xge_xpack_provider_t provider;
memset(&provider, 0, sizeof(provider));
provider.pPack = pack;
provider.sRoot = "assets";
provider.read = XPackRead;
provider.free = XPackFree;
xgeResourceXPackProviderAdd(&provider);
```

这样上层仍然可以用统一路径读取资源。

## 常见错误

| 问题 | 原因 | 处理方式 |
| --- | --- | --- |
| 相对路径在不同目录启动时失败 | 依赖 cwd | 使用 exe 根目录相对路径。 |
| `res://` 失败 | 没注册 provider | 启动时注册 resource provider。 |
| 数据泄漏 | 忘记 `xgeResourceFree` | 成功加载后配对释放。 |

## 下一步

- 查完整函数看 [Resource API](../api/resource.md)。
- 想把加载放到后台，看 [异步资源加载入门](async-intro.md)。

[返回教程入口](README.md)
