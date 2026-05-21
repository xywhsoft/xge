# XUI TimelineView Spec

本 spec 用于跟踪 `TimelineView` 开发。`TimelineView` 是时间轴编辑控件，面向类似 Flash 编辑器的图层帧时间轴场景。本轮不维护旧实验实现兼容。

## 设计状态

- [x] 明确控件名为 `TimelineView`。
- [x] 明确 `TimelineView` 是独立控件，不继承 `TableView`。
- [x] 明确控件基于 `Widget + ScrollModel + ScrollFrame`。
- [x] 明确控件自持轻量 UI 模型，业务数据通过 `id` / `userData` / 回调关联。
- [x] 明确 TimelineView 不自建动态数组或链表，优先使用 XRT 容器。
- [x] 明确图层顺序使用 `xparray` 保存 layer 指针。
- [x] 明确 layer 对象单独分配，移动图层时只移动指针，不复制 layer 内部状态。
- [x] 明确临时连续值缓存如确实需要，使用 `xarray`。
- [x] 明确帧数据使用稀疏模型，普通空帧不存储。
- [x] 明确每个图层的显式帧使用 `xlist`，key 为 `frameIndex`。
- [x] 明确每个图层的 span 使用 `xlist`，key 为 `startFrame`。
- [x] 明确 selection 使用 `xlist`，key 为 `((int64)layerIndex << 32) | frameIndex`。
- [x] 明确帧类型：`empty`、`normal`、`key`、`blankKey`。
- [x] 明确过渡帧不做内建业务语义，统一用 `Span` 表达范围段。
- [x] 明确 span 不允许重叠，必须有起止帧，允许跨关键帧。
- [x] 明确 span type 支持预设枚举和自定义字符串。
- [x] 明确锁定图层禁止修改帧和 span，但允许修改图层名。
- [x] 明确隐藏图层只改变状态，舞台内容是否隐藏由业务处理。
- [x] 明确显示/锁定能力可以通过功能开关屏蔽。
- [x] 明确当前帧和选择集分离。
- [x] 明确支持单选、Shift 范围选择、Ctrl 追加/取消、拖拽框选。
- [x] 明确默认尺寸：图层栏 `160px`、帧宽 `12px`、行高 `22px`、标尺高度 `24px`。
- [x] 明确支持横向缩放，第一版不做纵向缩放。
- [x] 明确 `Changing` 事件可拦截，`Changed` 事件只通知。
- [x] 明确不纳入播放控制、撤销栈、复制粘贴、音频波形、文件夹图层等编辑器能力。
- [x] 新增 `docs/xui/timelineview.md`。
- [x] 新增 `docs/xui/timelineview-spec.md`。

## 实现任务

- [x] 新增 `xge_xui_timeline_view_t` 数据结构。
- [x] 新增 TimelineView C API 声明。
- [x] 新增 `src/xge_xui_timeline_view.c`。
- [x] 在 `src/xge_impl.c` 纳入 TimelineView 实现文件。
- [x] 实现 init/unit，正确初始化和释放 `arrLayers` xparray、per-layer frame xlist、per-layer span xlist、selection xlist。
- [x] 实现默认 metrics：layer header width、frame width、min/max frame width、row height、ruler height。
- [x] 实现 `ScrollModel + ScrollFrame` 接入，不手写滚动条。
- [x] 实现四区域布局：corner、ruler、layer list、timeline grid。
- [x] 实现 content size 计算：宽度由 `frameCount * frameWidth` 决定，高度由所有 layer height 决定。
- [x] 实现横向滚动同步 ruler 和 grid。
- [x] 实现纵向滚动同步 layer list 和 grid。
- [x] 实现可见 layer 和可见 frame 范围计算。
- [x] 实现图层 `xparray` 增删改查。
- [x] 实现图层 move up / move down。
- [x] 实现图层 stable id 分配。
- [x] 实现图层名称设置和读取。
- [x] 实现图层 visible / locked 状态。
- [x] 实现 visible / locked 功能开关。
- [x] 实现显式帧 set/get/clear。
- [x] 实现 key frame / blank key frame / normal frame 默认绘制。
- [x] 实现 span add/remove。
- [x] 实现 span set/get。
- [x] 实现 span 重叠检查。
- [x] 实现 span 起止帧合法性检查。
- [x] 实现 span 默认绘制。
- [x] 实现 current frame 设置和读取。
- [x] 实现 current frame 红色播放头绘制。
- [x] 实现点击帧切换 current frame。
- [x] 实现拖动播放头连续切换 current frame。
- [x] 实现 selection set 清空、单帧选择、范围选择、查询。
- [x] 实现 Shift 范围选择。
- [x] 实现 Ctrl 追加/取消选择。
- [x] 实现鼠标拖拽框选。
- [x] 实现拖拽 preview selection，鼠标释放后统一提交。
- [x] 实现 frame selection changed 事件。
- [x] 实现图层行 hover / selected 绘制。
- [x] 实现 frame cell hover / selected 绘制。
- [x] 实现 lock 状态阻止帧和 span 修改。
- [x] 实现 lock 状态仍允许图层重命名。
- [x] 实现图层名双击重命名。
- [x] 实现图层右键菜单。
- [x] 实现帧右键菜单。
- [x] 实现右键菜单根据 lock / feature flag / hit-test 禁用对应项。
- [x] 实现基础 keyboard 支持：方向键移动 current frame，Shift 扩展选择。
- [x] 实现 `ensureFrameVisible`。
- [x] 实现横向 zoom：设置 frame width 并 clamp 到 min/max。
- [x] 实现 `Ctrl + 鼠标滚轮` 以鼠标所在帧为锚点缩放时间轴。
- [x] 强化选中帧默认绘制，选中状态需要有明显填充和边框。
- [x] 选中帧边框按连续区间合并绘制，避免相邻选中帧边线叠加变粗。
- [x] 实现 tooltip：图层名、图标、帧、span 动态 tooltip。
- [x] 实现 hit-test：corner、ruler、playhead、layer row、layer name、visible icon、lock icon、frame cell、span、selection、scrollbar。
- [x] 实现自定义 renderer：layer、ruler、frame、span。
- [x] 实现 renderer state flags，包含 hover、selected、locked、hidden、current、key、blankKey、span。
- [x] 实现 renderer 返回已处理时跳过默认内容绘制。
- [x] 实现裁剪，内容不得绘制到滚动条区域。

## 事件任务

- [x] 实现 `currentFrameChanging`，可拦截。
- [x] 实现 `currentFrameChanged`。
- [x] 实现 `layerSelected`。
- [x] 实现 `layerNameChanging`，可拦截。
- [x] 实现 `layerNameChanged`。
- [x] 实现 `layerVisibleChanging`，可拦截。
- [x] 实现 `layerVisibleChanged`。
- [x] 实现 `layerLockedChanging`，可拦截。
- [x] 实现 `layerLockedChanged`。
- [x] 实现 `frameClicked`。
- [x] 实现 `frameDoubleClicked`。
- [x] 实现 `frameChanging`，可拦截。
- [x] 实现 `frameChanged`。
- [x] 实现 `frameSelectionChanged`。
- [x] 实现 `spanChanging`，可拦截。
- [x] 实现 `spanChanged`。
- [x] 实现 `contextMenuOpening`，可拦截。
- [x] 实现 `contextMenuCommand`。
- [x] 高频拖拽过程中不触发完整 selection changed，只在提交时触发。
- [x] 右键菜单在点击 owner 空白区域或空白区域触发 context menu 时关闭。
- [x] `Create Span` 支持使用当前图层连续帧选择作为 span 起止范围。

## API 验收

- [x] C API 不出现 `TimelineEditor`、`FrameList` 等并行命名。
- [x] C API 能设置 frame count、frame rate、current frame。
- [x] C API 能设置 layer header width、frame width、row height、ruler height。
- [x] C API 能切换 compact/full 滚动条。
- [x] C API 能启用/禁用 visible 和 lock 功能。
- [x] C API 能增删移动图层。
- [x] C API 能设置图层名、显示状态、锁定状态、高度、颜色。
- [x] C API 能设置 layer/frame/span user data。
- [x] C API 能设置、读取、清理显式帧。
- [x] C API 能添加、删除 span。
- [x] C API 能修改 span。
- [x] C API 能拒绝同层重叠 span。
- [x] C API 能清空选择、选择单帧、选择范围、查询选择数量。
- [x] C API 能设置事件回调和 renderer 回调。
- [x] C API 能刷新布局和绘制缓存。
- [x] C API 能确保指定帧和图层可见。

## XSON 验收

- [x] 新增 `timelineView` XSON 类型。
- [x] 支持 `frameCount`。
- [x] 支持 `frameRate`。
- [x] 支持 `currentFrame`。
- [x] 支持 `layerHeaderWidth`。
- [x] 支持 `frameWidth`。
- [x] 支持 `rowHeight`。
- [x] 支持 `rulerHeight`。
- [x] 支持 `scrollbarMode`：`compact` / `full`。
- [x] 支持 `showVisibilityFeature`。
- [x] 支持 `showLockFeature`。
- [x] 支持 `layers` 对象数组。
- [x] `layers` 支持 `id`、`name`、`visible`、`locked`、`height`、`color`。
- [x] `layers.frames` 支持 `frame`、`type`。
- [x] `layers.spans` 支持 `id`、`start`、`end`、`type`、`customType`、`label`、`color`。
- [x] 支持基础颜色字段。
- [x] 加载错误能准确定位字段路径。
- [x] XSON 不要求表达大型时间轴全量数据。

## 范例验收

- [x] 新增 `examples/xui_timelineview`。
- [x] 新增 `examples/xui_timelineview_xson`。
- [x] C 范例可编译运行。
- [x] XSON 范例可编译运行。
- [x] 范例覆盖多图层。
- [x] 范例覆盖图层重命名。
- [x] 范例覆盖图层 visible 状态。
- [x] 范例覆盖图层 locked 状态。
- [x] 范例覆盖隐藏/锁定功能开关。
- [x] 范例覆盖普通帧、关键帧、空关键帧。
- [x] 范例覆盖 span。
- [x] 范例覆盖 span 不重叠规则。
- [x] 范例覆盖 current frame 切换。
- [x] 范例覆盖播放头拖拽。
- [x] 范例覆盖单选、Shift 选、Ctrl 选、拖拽框选。
- [x] 范例覆盖横向滚动。
- [x] 范例覆盖纵向滚动。
- [x] 范例覆盖横向缩放。
- [x] 范例覆盖图层菜单。
- [x] 范例覆盖帧菜单。
- [x] 范例覆盖自定义 renderer。
- [x] 范例覆盖大帧数稀疏数据，不为每帧创建 widget。

## 验收标准

- 图层列表和时间轴网格纵向滚动必须同步。
- 标尺和时间轴网格横向滚动必须同步。
- 当前帧红线必须覆盖标尺和网格，并跟随横向滚动正确显示。
- 内容不能绘制到滚动条区域。
- 标尺、图层列表、网格和选择框必须裁剪在各自 viewport 内。
- 锁定图层不能修改帧和 span。
- 锁定图层仍可修改图层名。
- 关闭 visible/lock 功能后，默认 UI 不绘制对应图标，对应菜单项必须禁用或不显示。
- span 重叠必须被拒绝。
- span 起止帧越界必须被拒绝或 clamp，策略必须在 API 文档中明确。
- Shift 选择必须以选择锚点为起点。
- Ctrl 选择不得清空已有选择。
- 拖拽框选释放前不得触发大量 changed 事件。
- current frame 和 selection 必须是两套状态。
- 自定义 renderer 不能破坏默认裁剪和滚动。
- 大时间轴必须只绘制可见图层和可见帧范围。
- XSON 范例和 C 范例必须成对存在。
