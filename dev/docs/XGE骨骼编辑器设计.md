# XGE 骨骼编辑器设计

本文限定 `tools/SkeletonEdit` 的编辑器部分。骨骼运行时、资产编译器和二进制缓存不在本线程优先范围内，但编辑器的数据结构必须为后续运行时对接保留清晰边界。

## 目标

- 使用 XGE/XUI 原生实现，不依赖外部编辑器、外部格式或第三方运行时。
- 使用 DockPanel 布局，保持工具窗口可停靠、可重排、可保存布局的方向。
- 首屏尽量简单：左侧资源树，右侧属性表，中间是骨骼编辑区域，中间上方是时间轴。
- 整体包含菜单、工具栏和状态栏。
- 交互以“选择对象 -> 修改属性 -> 预览结果”为主，不把第一版做成复杂专业 DCC。

## 主界面

```text
MenuBar
Toolbar
DockPanel
  Left:   Asset Tree
  Center: Timeline
          Skeleton Canvas
  Right:  Properties
StatusBar
```

### 菜单

- File: New, Open, Save, Save As, Exit
- Edit: Undo, Redo, Delete
- View: Reset Layout, Frame All
- Skeleton: Add Bone, Add Slot, Add Attachment
- Animation: Add Animation, Add Key, Play, Stop
- Help: About

### 工具栏

第一版只保留高频动作：

- New
- Open
- Save
- Select
- Bone
- Slot
- Attachment
- Key
- Play

工具栏按钮只切换模式或执行命令，不显示长说明文字。后续可以替换为图标。

### 左侧资源树

资源树是用户进入工程的主入口，按固定分类组织：

```text
Project
  Skeletons
    hero.xskel
  Images
    hero_body.png
    hero_arm.png
  Atlases
  Animations
    idle
    walk
  Skins
    default
```

第一版行为：

- 单击节点更新状态栏和属性表。
- 双击骨骼文件打开到中间编辑区。
- 拖图片到画布创建 attachment 的能力后置。

### 中间上方时间轴

直接使用现有 `xge_xui_timeline_view_t` 控件。第一版按骨骼/slot 轨道展示：

```text
root
body
head
arm.L
arm.R
slot.body
slot.head
events
```

第一版能力：

- 显示 frame ruler。
- 显示关键帧。
- 设置当前帧。
- 选中帧后刷新属性表。

后续能力：

- 拖拽关键帧。
- 框选范围。
- 轨道显隐/锁定。
- 曲线编辑器作为底部或右侧可选面板，不进入首屏默认布局。

### 中间骨骼编辑区

编辑区是一个 XUI 绝对布局画布。第一版先实现骨架示意绘制和选择框，后续再接入真实运行时预览。

基础显示：

- 棋盘或网格背景。
- 原点十字。
- 骨骼线段。
- bone joint 圆点。
- 当前选中对象高亮。

基础工具：

- Select: 选择骨骼/slot/attachment。
- Bone: 创建骨骼。
- Slot: 创建 slot。
- Attachment: 绑定图片区域。

### 右侧属性表

直接使用现有 `xge_xui_property_grid_t` 控件。根据选择类型切换分类：

- Document: name, path, frameRate
- Bone: name, parent, x, y, rotation, scaleX, scaleY, length
- Slot: name, bone, attachment, color, drawOrder
- Attachment: name, type, image, pivot, src rect
- Keyframe: frame, value, curve

第一版只需要能显示默认文档属性和选中资源属性；写回命令系统后置，但接口设计必须按 command 写回。

### 状态栏

状态栏固定展示：

- Ready / 当前命令状态
- Project / 当前文件
- Mode / 当前工具模式
- Selection / 当前选择
- Frame / 当前帧

## 编辑器内部边界

```text
ske_app_t
  shell widgets
  dock layout
  asset tree
  property grid
  timeline view
  document
  selection
  command stack

ske_document_t
  project path
  skeleton path
  bones
  slots
  attachments
  animations
  dirty revision

ske_command_t
  type
  before
  after
```

画布和属性表都不直接改运行时对象。所有用户编辑必须生成 command，command 修改 document，再刷新预览对象和 UI。

## 自有文档格式

第一版使用 `XGE_SKEL_TEXT 1` 文本格式，不依赖外部 DCC 或第三方骨骼格式。格式只表达编辑器当前 document：

- `document`: name, frameRate, dirty, selection, active bone/slot/animation, next ids。
- `bone`: id, parent, name, local x/y, rotation, length, connected, visible。
- `slot`: id, bone, name, attachment, drawOrder, color, visible。
- `attachment`: id, bone, name, image, offset, rotation, scale, pivot, drawOrder。
- `animation`: id, name, frameStart, frameEnd, loop。
- `keyframe`: id, animation, bone, frame, captured local transform。

该格式是编辑器内部交换格式，后续运行时可以再增加专用二进制缓存，但不替代 `.xskel` 源文件。

## 第一阶段完成定义

- `tools/SkeletonEdit` 能编译生成 `skeleton_edit.exe`。
- 启动后出现菜单、工具栏、状态栏。
- DockPanel 默认布局正确：左资源树、右属性表、中间上方时间轴、中间下方骨骼编辑画布。
- 资源树有默认工程节点。
- 属性表显示默认 document/canvas 属性。
- 时间轴有默认骨骼轨道和关键帧示例。
- 画布显示网格、原点和示意骨骼。
- `--smoke --frames 2` 能启动并退出，输出关键 UI 创建状态。
