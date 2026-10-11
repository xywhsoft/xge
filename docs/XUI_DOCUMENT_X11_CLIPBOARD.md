# Document 首版的原生 X11 文本剪贴板

日期：2026-10-09。针对基础复制、剪切、粘贴修复实际数据丢失与崩溃；HTML/CSS、网络动态资源、完整 UIA 等高级范围继续冻结。

## 已修复

| 原行为 | 当前行为 |
| --- | --- |
| Rich VISUAL 和 GFM VISUAL 同时提交文本、Document 片段与 HTML，Linux 后端仅接受一个文本项，因此复制/剪切返回不支持 | 后端按公开契约选择第一个 UTF-8 文本项；纯文本成功复制和剪切，其他格式不在本轮扩大支持 |
| Sokol 的固定 1 MiB 缓冲区截断文本，结果型接口仍返回成功；2 MiB 剪切删除完整原文，外部进程只得到约 1 MiB | X11 文本缓存按实际长度扩容；分配/所有权失败返回错误，编辑器不把失败当作成功剪切 |
| 外部进程拒绝 UTF-8 请求或文本超过接收缓存时，空指针进入 `strlen` 导致崩溃 | 结果型接口保留读取错误，直接 UTF-8 接收按长度扩容；没有剪贴板所有者返回有效空文本，空粘贴无操作 |
| 超出服务器单次请求上限的写入误报成功，或可视图片选区只复制换行而剪切删除图片 | 在修改剪贴板或文档前拒绝超限传输与当前后端无法保存的对象片段；原文、选区、保存点和历史保持 |

实现位于 `src/xge_render.c`。继续使用现有 Sokol 的 X11 窗口、选择所有权和事件循环，没有修改第三方库或新增第二套 Document。类型、长度、内嵌 NUL 和返回属性在读取/写入时检查；失败不会被旧的 void 包装再次重试为截断写入。公开 API、代理 ABI、文档格式与统一历史保持。

X11 单次写入还受服务器请求长度约束；后端保守预留请求头、扩展长度及对齐空间，超出安全预算在取得所有权和修改缓存之前返回 `BUFFER_TOO_SMALL`。缓存扩容不等于能传输任意长度，不能在未实现 `INCR` 发送时误报成功。

Linux Document 适配层在 `src/xui_proxy_xge.c` 检查既有原生片段，私有遍历位于 `src/xui_document_fragment.c`，覆盖 Root→Paragraph 的整块选区和文字混合对象选区。包含 IMAGE、RULE、HTML、MATH、DIAGRAM 或 EXTENSION 的可视复制/剪切返回 `UNSUPPORTED`；不能把换行或图片 alt 当作对象成功保存。Markdown Source/Live 源码选区仍能复制其源码；原生文件保存保留结构。这是基础数据保护，没有扩展 HTML/CSS 转换或对象剪贴板协议。单一有效空文本写入仍成功。

## 可重复运行

Linux/WSL 根目录：

```sh
python3 test_xui/build_document_x11_clipboard_test.py
```

入口默认调用正常/default 构建；已有当前运行库时可指定：

```sh
python3 test_xui/build_document_x11_clipboard_test.py --runtime-dir /absolute/path/to/runtime --out-dir /absolute/path/to/new-results
python3 test_xui/build_document_linux_basic_suite.py --runtime-dir /absolute/path/to/runtime --with-x11-clipboard
```

需要正常 XGE Linux 编译环境以及 `Xvfb`、`xauth` 和 Mesa 软件 OpenGL。驱动分配新显示、启用独立认证并禁用 TCP；不连接调用者的显示或读取/覆盖用户剪贴板。测试结束停止自己创建的服务器并删除认证文件。软件渲染线程固定为一，避免测试地址空间被 Mesa 的默认线程池占满；这不是产品性能配置。

`xui_document_x11_clipboard_test.c` 的窗口/字体使用确定性 TestProxy，四个剪贴板回调则是真实 XGE 后端。独立 Xlib 子进程持有并请求剪贴板，以完整字节比较验证跨进程交付，共34场景、493断言：

- Rich VISUAL、GFM VISUAL、SOURCE、LIVE 四配置各复制、剪切、粘贴与有效空粘贴。
- Unicode 选区、精确 Markdown 源码、revision、保存点、Undo/Redo。
- 2 MiB + 17 字节的实际接收、复制及剪切，精确内容与大文本历史往返。
- 外部拒绝 UTF-8、没有所有者、首个文本项位于第二项、没有文本项及含 NUL 的非法写入；错误保留文档或原剪贴板内容。
- 超过 X11 整个请求长度上限的写入和 Source Cut：明确拒绝，原文、选区、revision、保存点、历史与外部剪贴板保持。测试从服务器读取协议上限，不依赖产品中预留头部的预算公式。
- 整段图片、文字混合图片、分隔线选区各 Copy/Cut：返回不支持，保持文档和此前外部剪贴板；有效单一空文本写入仍能被外部进程精确读回。

基础套件默认仍执行原八组/25条命令；添加 `--with-x11-clipboard` 后是九组/26条外层命令，原生驱动内部另有四条命令。结果目录必须为新目录，记录实际退出码、日志与运行库哈希，并用 `ldd` 核对所加载的库。

## 实际验证

证据根目录：`build/source-document-footnote-local-reference-20261006`。

| 记录 | 结果 |
| --- | --- |
| `document-x11-clipboard-negative-v1040/linux-proof.json` | 原库复制/剪切不支持；2,097,169字节写入后外部实际仅1,048,575字节，剪切已删除完整文本；拒绝 UTF-8 后实际 SIGSEGV/-11 |
| `document-x11-clipboard-positive-v1042/linux-proof.json` | 当前来源正常137全新/零复用构建通过；原生长剪切遇到测试地址空间不足，整次原生检查不计通过 |
| `document-x11-clipboard-positive-v1043/linux-proof.json` | 诊断确认剪切前 VmSize 为2,095,892 KiB，接近测试2 GiB限制；原文仍保留 |
| `document-x11-clipboard-positive-v1044/proof.json` | 保持相同产品、同一2 GiB限制，只固定 Mesa 线程数；原21场景/285断言全部通过，外部完整读回大文本 |
| `document-x11-permanent-negative-v1045/linux-proof.json` | 新永久回归对旧库原生运行实际-11，能再次捕获缺陷；服务器/认证已清理，外层按预期负例检查退出0 |
| `document-x11-permanent-positive-v1046/linux-proof.json` | 当前库九组基础回归26条外层命令全部实际0；永久原生25场景/342断言通过；运行库与来源保持 |
| `document-x11-windows-baseline-v1047/proof.json` | 当前 Windows 全部18基础入口、3原控件消费者与文件/关闭内部18条命令通过；主库正常增量1编译/136复用，基础 WebView 同步复验 |
| `document-linux-native-collected-v1049/proof.json` | 新库重建Linux原生示例，八条成功命令实际0、两项无效字体检查预期1；8图像与此前核对版本一致 |
| `document-x11-transfer-limit-v1050/preprocess-proof.json` | 最后长度保护仅改Linux条件分支，Windows普通版与实际WebView2配置的编译器预处理结果逐字节相同 |
| `document-x11-permanent-positive-v1052/linux-proof.json` | 长度保护版本正常137全新/零复用构建；九组26条基础命令及安全预算边界回归通过 |
| `document-transfer-limit-windows-v1053/proof.json` | 当前普通版与WebView2均1增量编译/136复用；原生剪贴板、298导出、四示例检查和基础WebView等8入口实际0；复用未改变Windows代码的v1047完整18入口/3消费者证据 |
| `document-x11-permanent-positive-v1054/linux-proof.json` | 仅将最后两测试的输入加强为超过服务器整个请求上限，产品未变；27场景/376断言及4条驱动命令全部实际0 |
| `document-linux-native-positive-v1055/proof.json` | 长度保护库重新构建Linux示例，10条命令达到预期，8图像与此前版本一致，服务器与认证清理 |
| `document-x11-permanent-negative-v1056/linux-proof.json` | 加强回归对扩容已修复但缺少长度保护的前库运行1，超上限复制误报成功、剪切改变原文/选区/保存点/历史的7个断言失败 |
| `document-x11-permanent-negative-v1058/linux-proof.json` | 对已有长度保护库新增图片整段 Copy/Cut 回归，实际运行1；误报成功和剪切丢失原文等7个断言失败 |
| `document-x11-permanent-positive-v1059/proof.json`、`document-x11-permanent-positive-v1064/proof.json` | 两个开发候选未解决整段图片问题，外层实际1；目录名称不表示通过，这两次不接受 |
| `document-x11-structured-fence-v1068/preprocess-proof.json` | 最终Linux专用保护在Windows普通/WebView2配置下，xge.c、proxy和fragment三个单元的预处理结果均与原基线逐字节相同 |
| `document-x11-permanent-positive-v1069/linux-proof.json` | 当前来源正常137全新/零复用构建；九组26条基础命令全部0，原生34场景/493断言及内部4条命令全部0，服务器/认证已清理 |
| `document-transfer-limit-windows-v1070/proof.json` | 当前普通构建34编译/103复用，8项受影响入口全部0，298公开导出正确、14内部浏览器符号不公开；未重复未变化Windows代码的v1047完整基础矩阵 |
| `document-linux-native-positive-v1071/proof.json` | 当前Linux库重建示例，八项成功检查实际0、两项缺失字体检查预期1；8图像与此前核对版本一致，服务器/认证已清理 |
| `document-v1-v1073-x11-clipboard-acceptance.json` | 最终1720项来源/规则/文档、实际日志、运行库、公开导出与冻结产物独立核对 |

v1039编译夹具签名错误、v1041正常构建驱动90秒超时、v1042/v1043测试地址空间不足及v1048收集脚本末尾旧字段 KeyError均保留原记录。v1048的10条产品命令已实际达到预期，导出也完成；v1049独立核对后接受，不将原外层退出1改写成通过。正常构建限时随后单独延长，原生运行限时与产品断言保持。

两个图片保护候选分别只检查空文本或直接单图片容器，未覆盖整块选区的 Root→Paragraph 包装。最终改为递归检查完整片段，并增加混合图片/分隔线选区回归；没有放宽期望错误、原文保留或剪贴板保留断言。此前草稿引用的v1057收集记录未执行，以v1073最终核对为准。

Linux 当前正常库 SHA-256：`461bc8492ae49a99377a055df2e652c998e878fbde9fe7a20ae4cf0373a13521`。Windows当前普通库为`f36244872384ad7a88bec9a608a6b11b0434d6de77ccdf0c4f9716485560352d`。首版基础实现可供接入试用，本轮不宣称整体高级设计完成。

## 边界

本轮证明私有显示上的原生 X11 UTF-8 跨进程成功路径；不代替人工桌面跨应用使用、物理 GPU、真实 IME/键鼠、Linux文件对话框或 Windows 剪贴板成功路径。Windows 原生失败保护和 TestProxy 成功路径已复验，macOS/Android/iOS未新增平台证据。

X11 `INCR` 增量传输、旧 `STRING` 编码回退、复杂对象/图片与跨应用富格式尚未实现本轮协议支持；当前遇到未支持的传输类型明确失败并保留文档，不使用旧缓存或当作空粘贴。需要这些能力时单独完善。Windows基础WebView范围、其他四平台留空以及Document内部既有公式/Mermaid静态渲染的决定保持。
