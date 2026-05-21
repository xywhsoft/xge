# XUI TimelineView

`TimelineView` 是时间轴编辑控件，面向类似 Flash 编辑器时间轴的场景。它负责图层列表、帧标尺、时间轴网格、当前帧指示器、帧/范围段显示、选择、图层名称和显示/锁定状态；它不负责动画插值、播放控制、撤销栈、复制粘贴、音频波形或具体业务数据解释。内建重命名编辑器和右键菜单属于控件基础交互能力。

本文是 `TimelineView` 的唯一设计口径。后续代码、XSON、范例和 API 都以本文为准，不为旧实验实现保留兼容。

## 职责边界

- `Widget` 负责基础事件、焦点、裁剪、Z 序、tooltip、捕获和基础绘制。
- `ScrollModel` 保存时间轴内容尺寸、viewport 尺寸和滚动偏移。
- `ScrollFrame` 负责横纵滚动条、滚轮、滚动条拖拽、viewport 裁剪和滚动条模式。
- `TimelineView` 负责图层模型、帧稀疏模型、span 模型、标尺、网格、命中测试、选择、当前帧和默认绘制。
- 业务层负责解释帧、关键帧、span 和图层状态的业务含义。

`TimelineView` 自己持有轻量 UI 模型，业务数据通过 `id`、`userData` 或事件回调关联。控件可以独立完成滚动、选择、渲染、重命名和基础状态切换，不要求业务层为每个帧或图层创建 widget。

## 非目标

- 不做 Flash 完整动画系统。
- 不做自动补间、插值计算或关键帧求值。
- 不做播放控制按钮。
- 不做撤销栈。
- 不做复制粘贴。
- 不做音频波形。
- 不做文件夹图层。
- 不做多时间轴嵌套。
- 不做复杂编辑器级剪辑操作。

这些能力后续也不纳入默认开发清单，除非有明确新需求。

## 默认值

- 左侧图层栏宽度：`160px`。
- 帧宽：`12px`。
- 最小帧宽：`6px`。
- 最大帧宽：`40px`。
- 图层行高：`22px`。
- 帧标尺高度：`24px`。
- 默认滚动条模式：`compact`。
- 默认支持图层可见状态：启用。
- 默认支持图层锁定状态：启用。
- 默认当前帧：`0`。
- 默认总帧数：`120`。
- 默认选择模式：二维帧选择。

## 布局

时间轴由四个区域组成：

```text
corner          frame ruler
layer list      timeline grid
```

- `corner`：左上角控制区域，可显示全局状态、菜单入口或留空。
- `frame ruler`：顶部帧标尺，随横向滚动同步。
- `layer list`：左侧图层列表，随纵向滚动同步。
- `timeline grid`：右下时间轴网格，横纵方向都可滚动。

横向滚动只影响 `frame ruler` 和 `timeline grid`。纵向滚动只影响 `layer list` 和 `timeline grid`。当前帧指示器覆盖在 `frame ruler` 和 `timeline grid` 之上，不被图层内容遮盖。

`TimelineView` 不为每个帧格创建子 widget。绘制和命中测试都基于可见范围按需计算。

## 数据模型

### Timeline

时间轴根状态：

- `frameCount`：总帧数。
- `frameRate`：显示用帧率，控件不执行播放。
- `currentFrame`：当前帧，和 selection 分离。
- `layerHeaderWidth`：图层栏宽度。
- `frameWidth`：当前帧宽，支持横向缩放。
- `rowHeight`：默认图层行高。
- `rulerHeight`：标尺高度。
- `showVisibilityFeature`：是否启用图层显示/隐藏功能。
- `showLockFeature`：是否启用图层锁定功能。

### XRT 容器口径

TimelineView 不自建动态数组或链表，优先使用 XRT 容器：

- 图层顺序使用 `xparray_struct arrLayers`，数组元素保存 layer 指针。
- 每个 layer 对象单独分配，避免移动图层时复制包含 `xlist` / 缓存 / 字符串等内部状态的结构体。
- 显式帧使用 `xlist_struct frames` 或 `xlist frames`。
- span 使用 `xlist_struct spans` 或 `xlist spans`。
- selection 使用 `xlist_struct selection` 或 `xlist selection`。
- 临时可见项缓存、命中候选缓存等连续值数组如确实需要，使用 `xarray_struct`。

除非性能分析证明 XRT 容器无法满足需求，否则不引入 TimelineView 私有容器。

### Layer

图层顺序使用 `xparray` 保存。图层 `id` 保持稳定，数组 index 只表示当前显示顺序。

字段：

- `id`：稳定图层 ID。
- `name`：图层名称。
- `visible`：是否显示，语义由业务层处理。
- `locked`：是否锁定。
- `selected`：图层选择状态。
- `height`：行高，默认使用 timeline row height。
- `color`：图层辅助颜色。
- `userData`：业务数据指针。
- `frames`：该图层显式帧表，使用 `xlist`。
- `spans`：该图层范围段表，使用 `xlist`。

图层顺序操作使用 `xrtPtrArrayInsert`、`xrtPtrArrayAppend`、`xrtPtrArrayRemove`、`xrtPtrArraySwap` 等 XRT 指针数组能力，不使用 `xlist`。原因是图层需要按行号快速访问、上移、下移和稳定绘制顺序，指针数组比有序表更直接，也不会在移动时复制 layer 内部状态。

### Frame

帧数据使用稀疏模型。普通空帧不存储，只有被显式设置的帧才进入图层的 `frames` 表。

每个图层持有一个 `xlist`：

```text
key   = frameIndex
value = timeline_frame_t
```

帧类型：

- `empty`：默认空帧，不需要存储。
- `normal`：显式普通帧。
- `key`：关键帧。
- `blankKey`：空关键帧。

字段：

- `frameIndex`：帧号。
- `type`：帧类型。
- `selected`：可选缓存状态，权威选择状态仍在 selection set。
- `userData`：业务数据指针。

### Span

过渡帧不作为内建业务语义处理。`TimelineView` 使用 `Span` 表达范围段，业务层自行解释 span 是补间、动画片段、事件段、音频段或其他内容。

每个图层持有一个 `xlist`：

```text
key   = startFrame
value = timeline_span_t
```

规则：

- 同一图层内 span 不允许重叠。
- span 必须有起止帧。
- `startFrame <= endFrame`。
- span 允许跨关键帧。
- span type 支持枚举值和自定义字符串。
- span 只负责显示、命中和选择，不负责业务求值。

字段：

- `id`：稳定 span ID。
- `startFrame`：起始帧。
- `endFrame`：结束帧。
- `type`：预设类型。
- `customType`：自定义类型字符串。
- `label`：显示文本。
- `color`：显示颜色。
- `selected`：可选缓存状态，权威选择状态仍在 selection set。
- `userData`：业务数据指针。

预设 span type：

- `custom`
- `motion`
- `shape`
- `event`
- `audio`
- `hold`

这些 type 只影响默认显示风格，不携带业务行为。

### Selection

当前帧和选择集分离。点击帧通常同时改变 current frame 和 selection，但 API 上二者是两套状态。

选择集使用 `xlist` 存储：

```text
key = ((int64)layerIndex << 32) | frameIndex
```

value 可以是很小的结构体，用于保存选择状态或选择来源。由于图层 index 会因移动而变化，移动图层后需要重建 selection key；如果业务需要跨移动稳定选择，可以额外通过 layer id 重映射。

第一版支持：

- 单帧选择。
- `Shift` 范围选择。
- `Ctrl` 追加或取消选择。
- 鼠标拖拽框选。
- 离散 frame cell 集合选择。

第一版不支持复杂多矩形范围合并语义。拖拽过程中可以维护 preview selection，鼠标释放时统一提交并触发 selection changed。

## 图层状态

### 锁定

锁定主要用于避免业务内容被误修改。

默认规则：

- 锁定图层仍可选中。
- 锁定图层仍可切换当前帧。
- 锁定图层不允许修改帧和 span。
- 锁定图层允许修改图层名，因为重命名不修改帧内容，且操作路径明确。
- 锁定图层右键菜单中的帧/span 修改项禁用。

业务可以通过事件进一步拦截或放宽规则。

### 隐藏

隐藏只改变图层状态和默认图标显示。是否隐藏舞台内容由业务层处理。`TimelineView` 不因为 `visible = false` 自动隐藏该图层行，也不自动改变帧数据。

### 功能开关

显示/锁定功能都可以关闭。关闭后：

- 不绘制对应图标。
- 对应右键菜单项禁用或不显示。
- 不触发对应状态切换事件。
- API 仍可以保存字段值，但默认交互不暴露该功能。

## 交互

### 当前帧

- 单击帧格：切换当前帧，并默认选择该帧。
- 拖动播放头：连续切换当前帧。
- `Changing` 事件可拦截。
- 鼠标释放或最终确认后触发 `Changed` 事件。

### 帧选择

- 单击：选择一个帧。
- `Shift + 单击`：从锚点到当前帧形成矩形范围选择。
- `Ctrl + 单击`：追加或取消当前帧选择。
- 拖拽：框选多个图层和帧。
- 拖拽过程中默认不触发完整 changed 事件。
- 鼠标释放后触发一次 selection changed。

### 缩放

- `Ctrl + 鼠标滚轮`：以鼠标所在标尺/时间轴位置为锚点缩放帧宽。
- API 缩放：调用 `xgeXuiTimelineViewSetFrameWidth` 设置帧宽，内部会 clamp 到 `minFrameWidth/maxFrameWidth`。

### 图层

- 单击图层行：设置 active layer。
- 双击图层名：进入重命名。
- 右键图层行：弹出图层菜单。
- 点击眼睛图标：切换显示状态。
- 点击锁头图标：切换锁定状态。

眼睛和锁头是高频操作，因此默认允许直接点击。右键菜单提供同等操作。

### 右键菜单

图层菜单默认项：

- `Rename`
- `Visible`
- `Locked`
- `Add Layer`
- `Delete Layer`
- `Move Up`
- `Move Down`

帧菜单默认项：

- `Insert Frame`
- `Insert Keyframe`
- `Insert Blank Keyframe`
- `Clear Keyframe`
- `Create Span`
- `Clear Span`

菜单项是否启用需要结合命中目标、锁定状态、功能开关和业务拦截结果。

`Create Span` 在当前图层存在连续帧选择，且右键命中的帧属于该选择时，使用选择范围作为 span 的起止帧；否则使用命中帧开始的默认短 span。空白区域右键或点击会关闭已打开菜单。

## 事件

事件分为可拦截的 `Changing` 和只通知的 `Changed`。

可拦截事件：

- `currentFrameChanging`
- `layerNameChanging`
- `layerVisibleChanging`
- `layerLockedChanging`
- `frameChanging`
- `spanChanging`
- `contextMenuOpening`

通知事件：

- `currentFrameChanged`
- `layerSelected`
- `layerNameChanged`
- `layerVisibleChanged`
- `layerLockedChanged`
- `frameClicked`
- `frameDoubleClicked`
- `frameSelectionChanged`
- `frameChanged`
- `spanChanged`
- `contextMenuCommand`

事件参数需要至少包含：

- timeline widget。
- layer index。
- layer id。
- frame index 或 frame range。
- span id。
- old value。
- new value。
- modifiers。
- hit-test 信息。
- user data。

拖拽选择的实时预览事件默认不启用。如果后续确实需要实时反馈，可增加 `selectionPreviewChanged`，但必须由开关启用，避免高频事件拖慢复杂时间轴。

## 渲染

默认渲染必须完整可用，自定义渲染只是高级扩展。

默认视觉：

- 浅色整体背景。
- 左侧图层列表和右侧时间轴网格风格统一。
- 图层行轻微交替底色。
- 帧网格使用细线。
- 每 5 帧或 10 帧显示更明显的标尺刻度。
- 当前帧使用红色竖线和顶部红色播放头。
- 选择区域使用半透明蓝色。
- 关键帧使用实心圆或菱形。
- 空关键帧使用空心圆。
- span 使用浅色条带、起止边界和可选 label。
- 锁定图层降低可编辑提示，不降低文字可读性。
- 隐藏图层只改变眼睛图标状态。

自定义渲染回调：

- `layerRenderer`
- `rulerRenderer`
- `frameRenderer`
- `spanRenderer`

自定义 renderer 返回已处理时，默认内容绘制不再执行；`TimelineView` 仍负责裁剪、滚动、命中、当前帧覆盖层和选择状态维护。

## 命中测试

命中结果需要区分：

- `none`
- `corner`
- `ruler`
- `playhead`
- `layerRow`
- `layerName`
- `layerVisibilityIcon`
- `layerLockIcon`
- `frameCell`
- `span`
- `selection`
- `hScrollbar`
- `vScrollbar`

命中测试必须基于 `ScrollFrame` 的 viewport 和 `ScrollModel` 坐标转换，不允许在控件内部重复写一套滚动偏移算法。

## XSON 口径

XSON 类型名：

```text
timelineView
```

建议字段：

- `frameCount`
- `frameRate`
- `currentFrame`
- `layerHeaderWidth`
- `frameWidth`
- `rowHeight`
- `rulerHeight`
- `scrollbarMode`
- `showVisibilityFeature`
- `showLockFeature`
- `layers`
- `selection`
- `colors`

`layers` 子字段：

- `id`
- `name`
- `visible`
- `locked`
- `height`
- `color`
- `frames`
- `spans`

`frames` 子字段：

- `frame`
- `type`

`spans` 子字段：

- `id`
- `start`
- `end`
- `type`
- `customType`
- `label`
- `color`

XSON 主要用于范例和小型静态时间轴。大型时间轴仍建议通过 C API 构建或后续 adapter/binder 注入，避免超大 XSON 文件。

## API 口径

基础 API：

- `xgeXuiTimelineViewInit`
- `xgeXuiTimelineViewUnit`
- `xgeXuiTimelineViewClear`
- `xgeXuiTimelineViewSetFont`
- `xgeXuiTimelineViewSetFrameCount`
- `xgeXuiTimelineViewGetFrameCount`
- `xgeXuiTimelineViewSetFrameRate`
- `xgeXuiTimelineViewGetFrameRate`
- `xgeXuiTimelineViewSetCurrentFrame`
- `xgeXuiTimelineViewGetCurrentFrame`
- `xgeXuiTimelineViewSetMetrics`
- `xgeXuiTimelineViewSetFrameWidth`
- `xgeXuiTimelineViewSetLayerHeaderWidth`
- `xgeXuiTimelineViewSetRowHeight`
- `xgeXuiTimelineViewSetRulerHeight`
- `xgeXuiTimelineViewSetFeatureFlags`
- `xgeXuiTimelineViewSetScrollbarMode`
- `xgeXuiTimelineViewGetScrollbarMode`
- `xgeXuiTimelineViewSetColors`
- `xgeXuiTimelineViewEnsureFrameVisible`
- `xgeXuiTimelineViewHitTest`

图层 API：

- `xgeXuiTimelineViewAddLayer`
- `xgeXuiTimelineViewRemoveLayer`
- `xgeXuiTimelineViewMoveLayer`
- `xgeXuiTimelineViewGetLayerCount`
- `xgeXuiTimelineViewGetLayer`
- `xgeXuiTimelineViewSetLayerName`
- `xgeXuiTimelineViewSetLayerVisible`
- `xgeXuiTimelineViewSetLayerLocked`
- `xgeXuiTimelineViewSetLayerHeight`
- `xgeXuiTimelineViewSetLayerColor`
- `xgeXuiTimelineViewSetLayerUserData`
- `xgeXuiTimelineViewGetLayerUserData`

帧 API：

- `xgeXuiTimelineViewSetFrameType`
- `xgeXuiTimelineViewGetFrameType`
- `xgeXuiTimelineViewClearFrame`
- `xgeXuiTimelineViewSetFrameUserData`
- `xgeXuiTimelineViewGetFrameUserData`

Span API：

- `xgeXuiTimelineViewAddSpan`
- `xgeXuiTimelineViewRemoveSpan`
- `xgeXuiTimelineViewSetSpan`
- `xgeXuiTimelineViewGetSpan`
- `xgeXuiTimelineViewSetSpanUserData`
- `xgeXuiTimelineViewGetSpanUserData`

`xgeXuiTimelineViewAddSpan` 和 `xgeXuiTimelineViewSetSpan` 都显式接收 `startFrame/endFrame`，业务侧可以直接指定 span 范围。

选择 API：

- `xgeXuiTimelineViewClearSelection`
- `xgeXuiTimelineViewSelectFrame`
- `xgeXuiTimelineViewSelectRange`
- `xgeXuiTimelineViewIsFrameSelected`
- `xgeXuiTimelineViewGetSelectionCount`

回调 API：

- `xgeXuiTimelineViewSetCurrentFrameProc`
- `xgeXuiTimelineViewSetLayerProc`
- `xgeXuiTimelineViewSetFrameProc`
- `xgeXuiTimelineViewSetSpanProc`
- `xgeXuiTimelineViewSetLayerSelectedProc`
- `xgeXuiTimelineViewSetContextMenuProc`
- `xgeXuiTimelineViewSetSelectionProc`
- `xgeXuiTimelineViewSetFrameClick`
- `xgeXuiTimelineViewSetFrameDoubleClick`
- `xgeXuiTimelineViewSetRenderers`

具体函数名在实现时可以细化，但命名必须保持 `TimelineView` 唯一口径，不引入 `Timeline` / `TimelineEditor` / `FrameList` 等并行命名。
