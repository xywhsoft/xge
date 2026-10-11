# Document 块移动回归

入口使用统一 Document 内核，同时检查公开 `MoveBlockRange` 和 `MoveNode`。
336 个独立来源黄金覆盖实体/格式、Setext 标题、围栏、引用、隐藏链接定义、表格、列表、文件末尾、三种行结束、BOM 和正反 GAP 选区。检查源码、HTML、稳定节点 ID、旧快照以及 Undo/Redo。

另有 56 组逐分配点 OOM/合作取消扫描。失败不得改变发布状态、版本、保存状态、历史或通知；同一 Document 重试后检查共同历史、来源 Blob 共享及零泄漏。取消注入仅用于内核测试，使用私有取消令牌；源码黄金和命令均使用公开 API。

Windows（仓库根目录）：

```powershell
python test_xui/build_document_move_test.py --out-dir build/document-move-normal
python test_xui/build_document_move_test.py --out-dir build/document-move-collision --variant collision
```

Linux 默认启用 ASan/UBSan：

```sh
python3 test_xui/build_document_move_test.py --out-dir build/document-move-normal
python3 test_xui/build_document_move_test.py --out-dir build/document-move-collision --variant collision
```

使用 `--source-root` 指定独立候选源码；输出目录必须尚不存在。每次全新编译来源清单中的内核单元，固定输入散列并保存实际命令、退出码和日志。此入口目前在 Windows/Linux 验证，不替代完整 Document、真实 Editor、平台或性能验收。

## 实际共享库 Editor

已构建对应共享库时运行：

```sh
python test_xui/build_document_move_editor_test.py --build-dir build --out-dir build/document-move-editor
```

独立候选使用 `--source-root` 指定源码。252 个场景检查命令查询/执行、原字节、HTML、稳定 ID、选区、共同 Undo/Redo 与三模式往返。该入口使用真实 DLL 和代理绘制后端，不作为物理窗口、IME、DPI 或读屏验收。
