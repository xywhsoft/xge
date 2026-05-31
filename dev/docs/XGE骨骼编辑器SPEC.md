# XGE 骨骼编辑器 SPEC

状态说明：

- `[ ]` 未开始
- `[~]` 进行中
- `[x]` 已完成
- `[!]` 阻塞或需重新确认

## 1. 工程壳

- [x] 建立 `tools/SkeletonEdit` 工程目录。
- [x] 添加 Windows `build.bat`。
- [x] 添加 `src/main.c` 入口。
- [x] 添加 `src/core/ske_app.h` 和 `src/core/ske_app.c`。
- [x] 添加 `src/editor/ske_shell.c`。
- [x] 支持 `--frames N` 自动退出。
- [x] 支持 `--smoke` 冒烟模式。
- [x] 支持自有 `.xskel` 文本格式 roundtrip。

## 2. 主布局

- [x] 根布局使用 column。
- [x] 顶部添加 MenuBar。
- [x] MenuBar 下添加 Toolbar。
- [x] 中部添加 DockPanel host。
- [x] 底部添加 StatusBar。
- [x] DockPanel 默认左侧为 Asset Tree。
- [x] DockPanel 默认右侧为 Properties。
- [x] DockPanel 中间为编辑区。
- [x] 编辑区上方为 Timeline。
- [x] 编辑区下方为 Skeleton Canvas。

## 3. 初始控件

- [x] Asset Tree 使用 `xge_xui_tree_view_t`。
- [x] Properties 使用 `xge_xui_property_grid_t`。
- [x] Timeline 使用 `xge_xui_timeline_view_t`。
- [x] Skeleton Canvas 使用 XUI 画布 widget 和自定义 paint。
- [x] 状态栏显示 Ready、Project、Mode、Selection、Frame。

## 4. 简单交互

- [x] 点击资源树节点更新状态栏。
- [x] 工具栏选择 Select/Bone/Slot/Attachment/Key/Play 更新当前模式。
- [x] Timeline 当前帧变化更新状态栏。
- [x] 画布点击选中最近骨骼。
- [x] 属性表修改骨骼 name/length/rotation 并写回 document。
- [x] Add Bone 创建衔接到当前骨骼尾端的新骨骼。
- [x] Bone 模式在画布拖拽创建新骨骼。
- [x] Add Attachment 将当前图片资源绑定到当前骨骼。
- [x] 拖动骨骼尾端可调整骨骼长度和角度。
- [x] 点击附件占位矩形可选中附件。
- [x] 拖动附件占位矩形可调整附件 offset。
- [x] 属性表可修改附件 name/offset/rotation/drawOrder。
- [x] Delete 删除当前附件或无依赖的叶子骨骼。
- [x] Undo/Redo command stack。

## 5. 编辑器数据模型

- [x] 添加 `ske_document_t`。
- [x] 添加 bone/slot/attachment/animation 的编辑期结构。
- [x] 添加 selection model。
- [x] 添加 command model。
- [x] 添加 document dirty revision。

## 6. 骨骼编辑区

- [x] 绘制背景网格。
- [x] 绘制原点。
- [x] 绘制示意骨骼。
- [x] 绘制真实 document bones。
- [x] 支持 pan/zoom。
- [x] 支持选择高亮。
- [x] 支持创建骨骼。
- [x] 支持拖动骨骼尾端调整 length/rotation。
- [x] 支持拖动根骨骼起点移动整套连接骨骼。

## 7. 验证

- [x] `tools\SkeletonEdit\build.bat` 编译通过。
- [x] `tools\SkeletonEdit\release\skeleton_edit.exe --smoke --frames 2` 冒烟通过。
- [x] 静态编辑冒烟覆盖新增骨骼、slot、图片绑定、animation、keyframe、Undo、Redo、视图重置和 document roundtrip，summary 中 `bones=6 slots=1 attachments=1 animations=2 keys=1 dirty=1 undo=5 redo=0 zoom=1.00 pan=0.0,0.0 roundtrip=1`。
- [x] 后续接入真实 document 后补充 document roundtrip 测试。
