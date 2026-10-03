# XUI 通用 WebView：Windows W0 宿主探针

日期：2026-09-24。状态：W0 技术可行性已部分验证；此文保留独立探针结果。正式通用控件的后续实现与状态见 [XUI_WEBVIEW_WIDGET.md](XUI_WEBVIEW_WIDGET.md)。

独立设计见 `artifacts/xui-foundation-design/xui-webview-design.zh-CN.md`。本探针仅使用通用 HTML、textarea、导航、脚本与截图，不引用 Document、Markdown 或 MessageList 的业务类型。

## 已核实的事实

- 本机 WebView2 Runtime 为 `153.0.4234.48`；从官方 `Microsoft.Web.WebView2` NuGet 包提取 SDK `1.0.4191.47`。官方 [Win32 入门](https://learn.microsoft.com/en-us/microsoft-edge/webview2/get-started/win32)说明 SDK、环境、控制器和异步创建的关系。
- 所用 NuGet 包的 SHA256 为 `f492bbf547d0da329553b6727435b677579b1e9f91cc9e4a1ad029366d5f23d0`；包本身位于忽略的 `artifacts/` 验证目录，不纳入产品源码。
- SDK 的 `WebView2.h` 可以由当前 GCC/MinGW-w64 作为 C 接口编译；对供应商头使用 `-isystem` 后，本项目 `-Wall -Wextra -Werror` 可通过。SDK 的 `WebView2Loader.dll.lib` 可直接由该工具链链接，运行时使用同包 x64 Loader DLL。
- 独立 Win32 窗口和 `xgePlatformNativeHandle()` 返回的 XGE HWND 均可完成 STA 初始化、WebView2 异步环境/控制器创建、`NavigateToString`、导航完成通知和 `ExecuteScript`。HTML 的标题及 `<h1>` 内容由 DOM 脚本返回并核验。
- WebView2 的 `CapturePreview` 写入 `IStream`，生成实际 PNG：独立 Win32 窗口为 644×461，XGE 窗口右半区为 320×480。PNG 头、尺寸、输出字节及图片内容均已核查；该 API 的异步和内容加载边界见[官方接口说明](https://learn.microsoft.com/en-us/microsoft-edge/webview2/reference/win32/icorewebview2?view=webview2-1.0.4191.47)。
- 同一个 XGE HWND 中，XUI `TextEdit` 在左半区完成布局和真实像素绘制，WebView2 控制器承载右半区网页。XUI 输出和 WebView2 PNG 分别见 `artifacts/xui-webview-probe/capture-xui.png`、`capture-xge.png`。网页由原生子视图显示；这两张分别捕获的图不代表 XGE 纹理已合成浏览器像素。

## 复现

将官方 `Microsoft.Web.WebView2` NuGet 包解压，设置 `WEBVIEW2_SDK_DIR` 指向包含 `build/native` 的包根目录，然后在 Windows x64、GCC/MinGW-w64 和已安装 WebView2 Runtime 的环境执行：

```bat
call test_xui\build_webview_win32_probe.bat
```

脚本默认也接受本机忽略目录 `artifacts/xui-webview-probe/sdk`。它构建两个[探针](../test_xui/xui_webview_win32_probe.c)，分别运行独立 Win32 与 XGE/XUI 宿主。记录为 `artifacts/xui-webview-probe/validation-2026-09-24-w0-final2.log`；两次运行均以零退出码完成。

## 尚未通过的 W0 与后续验收

探针只证明当前机器上的 SDK/Runtime、C/COM/Loader、实际 HWND、网页绘制和基础 XUI 共存。它的静态 COM 回调对象只适合单进程探针。尚未验证真实中文 IME、XUI 与网页间键盘/焦点往返、剪贴板、窗口缩放、弹窗层级、重复创建销毁、运行时缺失、进程失败或辅助技术。因此设计 W0 的退出标准仍未完全满足，W1–W5、HTML 交互及 KaTeX/Mermaid provider 都是后续实际工作。
