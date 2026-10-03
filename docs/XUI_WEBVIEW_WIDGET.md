# XUI 通用 WebView 控件：首个产品实现

日期：2026-09-27。状态：Windows WebView2 后端已接入，部分基础能力经真实窗口验证；Document 已有静态公式/Mermaid/块级 HTML provider。长期任务进行中。

当前交付范围已收缩为 **Windows 基础网页控件**：加载并显示 URL/HTML/本地页面，处理 XUI 布局、裁剪、滚动、DPI、焦点、键鼠/中文 IME 和生命周期。其他四个原生平台后端暂留空；公开脚本、消息桥、请求/回复、截图及 W2–W5 的其余高级能力不再是基础版验收门槛。`xui.h` 已移除通信和截图声明；`src/xui_webview_internal.h` 保留 Document provider 使用的内部通道。正式 DLL 不导出该通道的符号，内部窗口专项使用显式开启测试导出的构建变体。范围依据见[WebView 设计修订](../artifacts/xui-foundation-design/xui-webview-design.zh-CN.md)。

2026-09-30 边界复核：正式 WebView2 DLL 的 `artifacts/xui-document-rebuild/validation-2026-09-30-webview-basic-boundary-exports.log` 证实 13 个内部浏览器符号均未导出；真实窗口基础控件测试 `validation-2026-09-30-webview-basic-widget.log` 通过。内部构建变体的 Document provider `validation-2026-09-30-webview-document-provider.log` 与离线渲染 `validation-2026-09-30-webview-document-render.log` 均通过，覆盖公式、Mermaid、HTML 的测量、绘制、截图和 MessageList 集成。随后增加的测试专用浏览器 PID 查询也只存在于内部构建变体，正式 DLL 导出表继续检查其缺席。因此公开控件的功能收缩不影响 Document 静态渲染通道。

控件入口为 [xui.h](../xui.h) 的 `xuiWebView*` API，实现在 [xui_webview.c](../src/xui_webview.c)。它只加载通用 URL 或自包含 HTML，与 Document、Markdown、MessageList 类型无依赖。普通 XUI DLL 总是包含 API；未启用 WebView2 时，控件仍可创建，但 `xuiWebViewInitializeAsync` 返回 `XUI_ERROR_UNSUPPORTED`。

## 可选构建与部署

在仓库根目录执行：

```bat
set WEBVIEW2_SDK_DIR=C:\path\to\Microsoft.Web.WebView2
set XUI_ENABLE_WEBVIEW2=1
call build_dll.bat default build\webview
```

`WEBVIEW2_SDK_DIR` 指向已解压的官方 NuGet 包根目录，其中有 `build/native/include/WebView2.h` 和 `build/native/x64/WebView2Loader.dll.lib`。运行时将同包的 `build/native/x64/WebView2Loader.dll` 与程序放在一起，并安装 WebView2 Runtime。未启用的正常构建不需要 SDK 或 Loader。当前是整 DLL 的可选构建变体，不是运行时插件；启用变体缺少 Loader DLL 时，操作系统可能无法加载整 DLL。

## 当前行为

- `xuiWebViewCreate` 只创建 XUI 控件；`InitializeAsync` 在实际 XGE Windows UI STA 线程建立原生子窗口、WebView2 environment/controller。状态在 `CREATED → INITIALIZING → READY/FAILED` 间转换；`READY` 事件之后可调用 `Navigate` 或 `LoadHtml`。
- 原生子窗口在可见帧跟随 XUI 世界矩形，并与窗口及 `HIDDEN/CLIP` 父容器求交。浏览器 viewport 保持控件的完整宽高，子窗口只呈现可见片段，不因裁剪而重排。隐藏、移出父树和更换根控件时立即同步隐藏原生宿主；这使用 XUI 新增的 `XUI_WIDGET_TYPE_UPDATE_ON_INACTIVE` 类型标志，隐藏子树平时不做额外逐帧遍历。
- `xuiWebViewFocus` 同时设置 XUI 焦点和浏览器原生焦点。浏览器的 GotFocus/LostFocus 与 XUI 焦点状态互相同步；XUI 焦点链进入网页时调用 WebView2 MoveFocus，网页发出 MoveFocusRequested 时尝试把焦点交给 XUI 的下一/上一控件。焦点回调以当前 HWND 和 XUI 焦点为准，过期的排队 Focus/Blur 不会将焦点反复拉回网页；宿主更新还会核对实际 HWND 焦点，使鼠标点击网页后 XUI 焦点也进入网页。隐藏、完全裁剪、移出父树或关闭时释放网页焦点；禁用 XUI 父容器时保留网页画面但禁用原生输入，重新启用后可再聚焦。
- `Close` 解绑事件、关闭 controller、销毁子窗口并发送一次 `CLOSED`。直接销毁控件不向已失效的 widget 发送事件。异步环境/控制器回调使用 COM 引用计数持有后端，并在关闭后拒绝把迟到的 controller 重新接上。
- WebView2 报告浏览器主进程或主框架渲染进程退出时，控件立即转为 `FAILED` 并取消待处理的内部脚本/截图请求；下一次 XUI 更新在 WebView2 回调栈外释放 controller 与原生宿主，并发送一次 `FAILED`，错误码为 `E_FAIL`。GPU、工具进程故障及单个子框架退出不关闭整个控件。调用方可销毁失败控件并创建新实例；目前不自动重建。控件脱离 XUI 更新树时，失败事件与宿主清理需等重新进入更新树或由调用方关闭/销毁。
- `xuiWebViewSetZoomFactor` / `xuiWebViewGetZoomFactor` 在 READY 状态操作实际 WebView2 controller，接受 0.25–5 倍页面缩放；缩放不改变 XUI 控件与原生宿主的矩形。真实窗口把固定 30px 的网页标记从 1 倍放大到 1.5 倍，PNG 中 40px 处从背景变为标记颜色；XUI 虚拟 DPI 调到 1.5 再恢复时宿主矩形保持原值。本机显示器报告 DPI 1.0，这些证据不能代替物理 150%/200% DPI 和跨屏移动验收。
- UTF-8 URL、HTML 和 profile 目录进入 WebView2 前转换为 UTF-16。描述中的目录在创建时复制；`NULL` 使用 WebView2 默认位置。`LoadHtml` 调用 `NavigateToString`，适合自包含页面，尚未提供相对资源、可配置 origin 或内容安全策略。
- 当前回调包括初始化就绪/失败、导航成功/失败、关闭。初始化失败保存 HRESULT；网页导航失败目前统一报告 `E_FAIL`，还没有公开 WebView2 的详细网络状态。调用方必须在 UI 线程操作该控件；跨线程调用没有调度保证。

## 本地资源与 Document 内部渲染通道

本地目录映射属于通用控件的公开基础功能。下述脚本、消息和截图接口仅在 `src/xui_webview_internal.h` 中声明，服务于 Document 的公式、Mermaid 等静态渲染与内部测试，不属于通用 WebView 的公开基础功能。正常构建的 DLL 导出表不包含这 13 个内部通道符号，也不包含测试专用的浏览器 PID 查询符号；`XUI_WEBVIEW_TEST_EXPORTS=1` 仅供内部跨 DLL 窗口测试使用。`test_xui/build_webview_exports_test.bat` 构建正式变体并用运行时导出查询核对公开与内部边界。

`xuiWebViewMapLocalFolder` 把应用提供的现存绝对目录映射到合法的 ASCII DNS 主机名；应用随后可导航到 `https://<host>/...`，用同源 URL 加载 HTML、CSS、JavaScript、图片等资源。`xuiWebViewUnmapLocalFolder` 撤销映射。跨源访问由 `SAME_ORIGIN`、`SUBRESOURCE`、`ALL` 三种明确策略决定，分别对应 WebView2 的 `DENY`、`DENY_CORS`、`ALLOW`；默认枚举值是同源限制。映射只服务调用方指定的目录，调用方应使用自己控制的虚拟主机名。已有页面修改映射后需重新导航。策略语义参见 [Microsoft 的资源访问说明](https://learn.microsoft.com/en-us/microsoft-edge/webview2/reference/win32/webview2-idl#corewebview2_host_resource_access_kind)。

`xuiWebViewEvalScriptAsync` 返回可独立于 widget 持有的请求。调用方在 UI 线程轮询 `GetState`，成功后借用 `GetResultJson` 的 UTF-8 JSON；显式取消、导航、关闭会把未完成请求置为 `CANCELLED`，迟到的 COM 回调不改写终态。取消只终止结果交付，不保证停止网页脚本或回滚其副作用；请求 `Release` 不隐式取消。脚本和结果各限 16 MiB，一个 WebView 同时最多接受 64 个底层执行回调尚未结束的脚本请求，超限返回 `XUI_ERROR_LIMIT_EXCEEDED`；取消结果后仍占用容量，直到底层回调结束。当前使用 WebView2 `ExecuteScript` 的 JSON 结果；`undefined` 等不可序列化结果可能是 JSON `null`，尚未区分脚本异常或等待 JavaScript Promise，也没有完成事件/回调。结果语义参见 [Microsoft 的 ExecuteScript 文档](https://learn.microsoft.com/en-us/microsoft-edge/webview2/reference/win32/icorewebview2#executescript)。

`xuiWebViewSetMessageHandler` 显式开启 JSON 消息桥；默认关闭。调用方指定一个 `http(s)://DNS-host` origin（当前不接受端口或路径），接收事件时同时核对事件来源与浏览器当前页面 URL，来自其他来源或 URL 已变化的旧页面消息直接丢弃。相同 URL 重载的跨页面代次尚不能区分，应用不可把此基础回调用作可靠请求回复协议。`xuiWebViewPostMessageJson` 只向当前允许的来源发送 JSON，输入最多 1 MiB，使用浏览器消息 API 而非拼接 JavaScript；在未注册或当前来源不匹配时返回错误。传入 `NULL` origin 与回调可关闭桥。回调借用来源和 JSON 字符串，仅在回调期间有效。这是主框架、单向通知/回显的基础能力；尚无通道名、请求 ID、超时、Promise 回复或子框架授权。来源校验和 JSON 消息的依据见 [Microsoft WebView2 安全说明](https://learn.microsoft.com/en-us/microsoft-edge/webview2/concepts/security) 与 [消息 API 文档](https://learn.microsoft.com/en-us/microsoft-edge/webview2/reference/win32/icorewebview2#postwebmessageasjson)。

`xuiWebViewCapturePngAsync` 在页面加载且宿主可见时异步截取完整浏览器 viewport；`xuiWebRequestGetResultPng` 借用成功请求持有的 PNG 字节。单边最多 4096 像素、面积最多 1600 万像素、同一视图最多 2 个底层截图，返回字节最多 64 MiB。导航、隐藏、关闭或销毁控件会取消待完成截图，已完成请求可在控件销毁后继续持有结果。WebView2 在完全隐藏时可能长期不调用完成回调，因此隐藏状态拒绝新截图；只裁剪到 1×1 可见像素仍保留网页完整 viewport，可供离线静态对象 worker 使用。截图不是 DOM 交互、选择或辅助技术接口。

## 验证与未完成项

`test_xui/build_webview_widget_test.bat` 在真实 XGE 窗口中构建启用后端的 DLL，验证 HTML 导航、裁剪保持 viewport、XUI 焦点链切入/切出、程序化聚焦、父级隐藏/恢复、完全裁剪、移动、移除/重挂、替换/恢复根控件、关闭、初始化中直接销毁及 Ready 回调内重入关闭/销毁。最终运行记录为 `artifacts/xui-webview-probe/validation-2026-09-24-focus-inactive-final.log`，零退出码。

焦点回授修复后的窗口测试还覆盖原生窗口失焦/恢复、父容器禁用/启用和焦点阻断。设置 `XUI_WEBVIEW_SYNTHETIC_TAB=1` 运行同一测试程序时，系统通过 `SendInput` 注入 Tab 与 Shift+Tab，分别验证网页到后一个/前一个 XUI 控件的原生焦点交接；记录为 `artifacts/xui-webview-probe/validation-2026-09-24-focus-tab-enabled-build.log` 与 `validation-2026-09-24-focus-tab-enabled-synthetic.log`，均为零退出码。注入式输入需要可交互的前台桌面，因此不作为无界面 CI 的默认路径。

设置 `XUI_WEBVIEW_SYNTHETIC_POINTER=1` 时，测试在原生 XUI 控件持有焦点后向网页注入真实鼠标点击，核对浏览器 HWND 与 XUI 焦点同步。与 `XUI_WEBVIEW_SYNTHETIC_TAB=1` 同时设置的组合流程（点击进入网页，再从网页双向 Tab 返回 XUI）串行运行三次均通过，见 `artifacts/xui-webview-probe/validation-2026-09-24-pointer-tab-serial-1.log` 至 `-3.log`。这些前台输入测试不能与其他打开窗口的测试并行运行。

`test_xui/build_webview_scroll_test.bat` 在真实 XUI ScrollView 内验证原生宿主与视口的矩形交集：部分滚动后网页保持可见及焦点，完全滚出视口后隐藏并释放焦点，滚回后恢复；记录为 `artifacts/xui-webview-probe/validation-2026-09-24-scroll-viewport-final.log`，零退出码。

`test_xui/build_webview_script_test.bat` 在真实页面中验证 JSON/Unicode/`null`、非法 UTF-8、显式取消、导航与关闭取消、结果跨 widget 销毁保留，以及 64 个实际执行回调上限；取消满队列后，立即提交仍被限额拒绝，底层回调完成后容量恢复。记录为 `artifacts/xui-webview-probe/validation-2026-09-24-inflight-script-final.log`。`test_xui/build_webview_local_test.bat` 用临时映射目录加载 HTML、CSS、JavaScript 和 SVG，再以脚本检查标题、计算样式、脚本变量和图片尺寸；页面与宿主互发 JSON，网页自行跳转到另一虚拟主机后两侧消息均被来源策略拒绝。记录为 `validation-2026-09-24-message-self-nav.log`，两项均零退出码。资源测试只证明同源加载和撤销映射调用；跨源资源策略矩阵、缓存与错误页面行为仍需专项验证。

`test_xui/build_webview_capture_test.bat` 在实际窗口中用 PNG 解码和中心像素检查普通截图、导航/关闭取消、隐藏取消及 1×1 可见裁剪下网页改色后的完整 viewport 截图，记录为 `artifacts/xui-webview-probe/validation-2026-09-24-capture-clipped-probe.log`。`test_xui/build_document_web_render_test.bat` 加载离线 KaTeX/Mermaid 页面并核对两张 PNG 的尺寸与非空像素；`test_xui/build_document_web_provider_test.bat` 把它们连接到真实 DocumentView，修改 Markdown 来源后核对重渲染，再绑定到 MessageList 检查气泡内绘制与消息行高，详见 [Document 验证记录](XUI_DOCUMENT_VALIDATION.md)。

普通 DLL 在不设置 SDK 的情况下完成构建；`xui_webview_disabled_test.c` 验证禁用时的公共 API 与 `XUI_ERROR_UNSUPPORTED`，DLL 导入表中也没有 WebView2/Edge 依赖。`xui_widget_inactive_update_test.c` 证明隐藏、脱离父树和更换根控件时各同步一次，而长期隐藏不增加逐帧更新。对同一普通 DLL 的 widget/type、Document 内核、renderer 和 editor 回归测试均通过。独立 W0 探针另见 [XUI_WEBVIEW_W0.md](XUI_WEBVIEW_W0.md)。

点击进入网页与 Tab/Shift+Tab 出网页已通过本机系统注入与 HWND 状态检查，页面缩放和 XUI 虚拟 DPI 的几何检查也通过。`XUI_WEBVIEW_SYNTHETIC_TEXT=1` 又把系统 `KEYEVENTF_UNICODE` 事件的中文字符与 ASCII 连续输入送到网页 textarea，并从 DOM 核对结果；与点击及双向 Tab 同时运行也通过，见 `artifacts/xui-webview-probe/validation-2026-09-25-unicode-pointer-tab.log`。这不是拼音 IME 预编辑、候选窗口或网页与原生编辑器之间连续输入的证据；物理高 DPI/跨屏和窗口缩放也未实测。同一真实窗口中连续 12 轮创建、导航、关闭与销毁已通过，每轮都核对网页完成加载、关闭事件与原生宿主窗口消失，轮次之间没有孤留宿主；记录为 `artifacts/xui-document-rebuild/validation-2026-09-30-webview-repeated-lifecycle-final.log`。同窗口双 WebView 同时加载后，关闭其一不影响另一实例再次导航，两个原生宿主均按顺序释放；最终实窗记录为 `artifacts/xui-document-rebuild/validation-2026-09-30-webview-concurrent-lifecycle-final.log`。独立临时 profile 的实窗测试终止经身份核对的测试专属浏览器进程，确认失败事件、原生宿主清理，以及重新创建后可再次导航，见 `artifacts/xui-document-rebuild/validation-2026-09-30-webview-process-failure-final-widget.log`。Document provider 在独立 profile 下的实窗测试还验证了浏览器主进程退出后，MessageList 把排队公式绘制为错误卡片，并在重建 provider 后恢复渲染，见 `artifacts/xui-document-rebuild/validation-2026-09-30-document-provider-process-integration-provider-probe2.log`。主框架渲染进程异常、运行时缺失、popup 层级、远程导航错误、更大规模或跨窗口并发与可访问性仍需专项验收。多 profile 共享/迁移策略、完整网页数据通信、文件/下载/权限等属于后续通用控件扩展，不再是当前 Windows 基础版完成门槛。Document 公式、Mermaid 和块级原始 HTML 已有静态 provider；HTML 浏览器内交互及高级对象的辅助技术和完整编辑体验尚未验收。
