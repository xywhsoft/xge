#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M4 扫尾脚本：枚举逐值/组注释、90 个 desc 结构体级注释、7 个核心 desc 逐字段、宏组注释。"""
import re
from pathlib import Path

p = Path('xui.h')
parts = re.split(r"(\r\n|\n|\r)", p.read_bytes().decode('utf-8'))
lines, seps = [], []
for j in range(0, len(parts), 2):
    lines.append(parts[j])
    seps.append(parts[j + 1] if j + 1 < len(parts) else "")

def commented(i):
    return i >= 0 and ("/*" in lines[i] or lines[i].strip().startswith(("*", "//")))

ins = []

RES = {"XUI_OK = 0": "/* 成功。 */", "XUI_ERROR = -1": "/* 通用失败。 */",
 "XUI_ERROR_INVALID_ARGUMENT = -2": "/* 参数非法（空指针/越界）。 */",
 "XUI_ERROR_NOT_INITIALIZED = -3": "/* 上下文或对象尚未初始化。 */",
 "XUI_ERROR_ALREADY_INITIALIZED = -4": "/* 重复初始化。 */",
 "XUI_ERROR_OUT_OF_MEMORY = -5": "/* 内存分配失败。 */",
 "XUI_ERROR_FILE_NOT_FOUND = -6": "/* 文件不存在。 */",
 "XUI_ERROR_UNSUPPORTED = -7": "/* 能力或代理后端不支持。 */",
 "XUI_ERROR_BACKEND_FAILED = -8": "/* 渲染后端操作失败。 */",
 "XUI_ERROR_GPU_FAILED = -9": "/* GPU/驱动操作失败。 */",
 "XUI_ERROR_RESOURCE_FAILED = -10": "/* 资源加载或解析失败。 */",
 "XUI_ERROR_BUFFER_TOO_SMALL = -11": "/* 输出缓冲不足（API 同时输出所需大小）。 */",
 "XUI_ERROR_LAYOUT_UNSTABLE = -12": "/* 布局未收敛（超过 XUI_LAYOUT_MAX_PASSES 轮）。 */",
 "XUI_ERROR_NOT_FOUND = -13": "/* 目标条目不存在。 */",
 "XUI_ERROR_INVALID_STATE = -14": "/* 当前状态不允许该操作。 */"}
for tok, c in RES.items():
    for i, l in enumerate(lines):
        if tok in l:
            ins.append((i, c)); break

for i, l in enumerate(lines):
    if "typedef enum xui_error_stage_t" in l:
        ins.append((i, "/* 错误发生阶段（错误信息定位用）。 */")); break
STG = {"XUI_ERROR_STAGE_GENERAL = 0": "/* 通用阶段。 */", "XUI_ERROR_STAGE_LAYOUT = 1": "/* 布局阶段。 */",
 "XUI_ERROR_STAGE_CACHE = 2": "/* 缓存阶段。 */", "XUI_ERROR_STAGE_RENDER = 3": "/* 渲染阶段。 */",
 "XUI_ERROR_STAGE_UPDATE = 4": "/* 更新阶段。 */", "XUI_ERROR_STAGE_INPUT = 5": "/* 输入阶段。 */",
 "XUI_ERROR_STAGE_USER = 6": "/* 用户自定义阶段。 */"}
for tok, c in STG.items():
    for i, l in enumerate(lines):
        if tok in l:
            ins.append((i, c)); break

for i, l in enumerate(lines):
    if "typedef enum xui_text_id_t" in l:
        ti = i
        ins.append((i, "/* 内建 UI 文案槽位表：经 xuiTranslate 取当前语言译文；自定义语言包可按槽位覆盖，\n * 未覆盖回落默认语言。值名即语义，分组见各组首注释。 */"))
        break
GRP = {"NONE": "/* 空槽位（自定义文案占位）。 */", "EDIT": "/* 编辑命令通用文案。 */",
 "FIND": "/* 查找与替换窗口文案（含结果列名与状态提示）。 */",
 "CODE": "/* 代码编辑器命令文案。 */", "DOCK": "/* 停靠面板窗口菜单文案。 */",
 "TERMINAL": "/* 终端命令文案。 */", "FILE": "/* 文件对话框文案（含覆盖确认）。 */",
 "RICH": "/* 富文本编辑工具栏文案（对齐/标题/列表）。 */",
 "TIMELINE": "/* 时间轴上下文命令文案。 */", "MESSAGE": "/* 消息列表命令文案。 */"}
end = next(i for i in range(ti, len(lines)) if lines[i].startswith("}"))
cur = None
for i in range(ti + 1, end):
    m = re.match(r"\s*(XUI_TR_([A-Z]+))", lines[i])
    if not m:
        continue
    seg = m.group(2)
    if seg == "REPLACE":
        seg = "FIND"
    if seg != cur:
        cur = seg
        c = GRP.get(seg)
        if c and not commented(i - 1):
            ins.append((i, c))

D = {
"xui_surface_desc_t": "/* surface 创建描述（代理后端消费）。 */",
"xui_text_layout_desc_t": "/* 文本布局创建描述（文本/宽度/换行/省略）。 */",
"xui_label_desc_t": "/* 标签创建描述；全部字段有默认值。 */",
"xui_hyperlink_desc_t": "/* 超链接创建描述。 */",
"xui_breadcrumb_desc_t": "/* 面包屑创建描述。 */",
"xui_image_desc_t": "/* 图片创建描述（surface/源区/适配模式）。 */",
"xui_qrcode_desc_t": "/* 二维码创建描述。 */",
"xui_panel_desc_t": "/* 面板创建描述（标题/图标/头高）。 */",
"xui_separator_desc_t": "/* 分隔线创建描述。 */",
"xui_flow_port_desc_t": "/* 流程图端口描述（方位/标签/类型）。 */",
"xui_flow_node_desc_t": "/* 流程图节点描述（类型/位置/端口）。 */",
"xui_flow_edge_desc_t": "/* 流程图连线描述（源/目标端口）。 */",
"xui_flow_move_node_desc_t": "/* 节点移动命令描述（撤销记录用）。 */",
"xui_flow_diagnostic_desc_t": "/* 流程图诊断描述（节点/连线错误）。 */",
"xui_flow_graph_desc_t": "/* 流程图画布创建描述。 */",
"xui_workflow_desc_t": "/* 工作流画布创建描述。 */",
"xui_workflow_node_type_desc_t": "/* 工作流节点类型描述（端口/配置 schema）。 */",
"xui_workflow_variable_desc_t": "/* 工作流变量描述。 */",
"xui_workflow_config_field_desc_t": "/* 工作流配置字段描述（schema 构件）。 */",
"xui_progress_desc_t": "/* 进度条创建描述。 */",
"xui_step_bar_desc_t": "/* 步骤条创建描述。 */",
"xui_chart_desc_t": "/* 图表创建描述。 */",
"xui_code_layout_desc_t": "/* 代码布局计算描述（文档/字体/视口输入）。 */",
"xui_code_margin_desc_t": "/* 代码边栏描述（id/宽度/可见）。 */",
"xui_code_edit_desc_t": "/* 代码编辑器创建描述。 */",
"xui_input_decoration_desc_t": "/* 输入框装饰描述（图标/按钮/间距）。 */",
"xui_input_desc_t": "/* 输入框创建描述；全部字段有默认值。 */",
"xui_tag_input_desc_t": "/* 标签输入创建描述。 */",
"xui_numeric_input_desc_t": "/* 数值输入创建描述（范围/步进/精度）。 */",
"xui_text_edit_desc_t": "/* 多行文本创建描述。 */",
"xui_rich_edit_desc_t": "/* 富文本编辑器创建描述。 */",
"xui_button_desc_t": "/* 按钮创建描述；全部字段有默认值，关键字段见各行注释。 */",
"xui_checkbox_desc_t": "/* 复选框创建描述。 */",
"xui_check_card_desc_t": "/* 勾选卡片创建描述。 */",
"xui_radio_desc_t": "/* 单选框创建描述。 */",
"xui_radio_group_desc_t": "/* 单选组创建描述。 */",
"xui_toggle_desc_t": "/* 开关创建描述。 */",
"xui_scrollbar_desc_t": "/* 滚动条创建描述。 */",
"xui_slider_desc_t": "/* 滑动条创建描述。 */",
"xui_range_slider_desc_t": "/* 范围滑动条创建描述。 */",
"xui_page_desc_t": "/* 分页器创建描述。 */",
"xui_inventory_grid_desc_t": "/* 物品栏创建描述（槽位数/度量/颜色）。 */",
"xui_terminal_desc_t": "/* 终端创建描述（行列/回滚上限/解析预算）。 */",
"xui_terminal_session_desc_t": "/* 终端会话描述（假会话基类）。 */",
"xui_terminal_process_desc_t": "/* 子进程会话描述（命令行/环境/编码）；Windows 专用。 */",
"xui_terminal_ssh_desc_t": "/* SSH 会话描述（主机/端口/认证）。 */",
"xui_carousel_desc_t": "/* 轮播创建描述。 */",
"xui_virtual_joystick_desc_t": "/* 虚拟摇杆创建描述。 */",
"xui_canvas_desc_t": "/* 画布创建描述。 */",
"xui_split_layout_desc_t": "/* 分割布局创建描述。 */",
"xui_tabs_desc_t": "/* 选项卡创建描述。 */",
"xui_accordion_section_desc_t": "/* 手风琴分区描述。 */",
"xui_accordion_desc_t": "/* 手风琴创建描述。 */",
"xui_window_desc_t": "/* 虚拟窗口创建描述；关键字段见各行注释。 */",
"xui_dock_panel_desc_t": "/* 停靠面板创建描述（度量/颜色组）。 */",
"xui_msgbox_desc_t": "/* 消息框创建描述（标题/消息/按钮/模态）。 */",
"xui_file_dialog_desc_t": "/* 文件对话框创建描述（模式/过滤/目录）。 */",
"xui_msgtip_desc_t": "/* 轻提示创建描述。 */",
"xui_toast_desc_t": "/* Toast 通知中心创建描述。 */",
"xui_scroll_frame_desc_t": "/* 滚动框架创建描述。 */",
"xui_scroll_view_desc_t": "/* 滚动视图创建描述。 */",
"xui_list_view_desc_t": "/* 列表视图创建描述。 */",
"xui_message_list_desc_t": "/* 消息列表创建描述。 */",
"xui_tree_view_desc_t": "/* 树视图创建描述。 */",
"xui_table_view_desc_t": "/* 表格视图创建描述。 */",
"xui_table_grid_desc_t": "/* 数据网格创建描述。 */",
"xui_timeline_view_desc_t": "/* 时间轴创建描述。 */",
"xui_property_desc_t": "/* 属性行描述（类型/名称/默认值/约束）。 */",
"xui_property_grid_desc_t": "/* 属性网格创建描述。 */",
"xui_popup_desc_t": "/* 弹出层创建描述；关键字段见各行注释。 */",
"xui_menu_desc_t": "/* 菜单创建描述；关键字段见各行注释。 */",
"xui_menubar_desc_t": "/* 菜单栏创建描述。 */",
"xui_toolbar_desc_t": "/* 工具栏创建描述。 */",
"xui_statusbar_desc_t": "/* 状态栏创建描述。 */",
"xui_combobox_desc_t": "/* 组合框创建描述。 */",
"xui_cascader_desc_t": "/* 级联选择器创建描述。 */",
"xui_color_picker_desc_t": "/* 颜色选择器创建描述。 */",
"xui_icon_picker_desc_t": "/* 图标选择器创建描述。 */",
"xui_date_picker_desc_t": "/* 日期选择器创建描述。 */",
"xui_state_style_desc_t": "/* 状态样式规则描述（状态位 + 属性表）。 */",
"xui_style_desc_t": "/* 样式规则描述（属性表载体）。 */",
"xui_resource_desc_t": "/* 命名资源注册描述。 */",
"xui_rich_image_desc_t": "/* 富文本图像描述。 */",
"xui_rich_table_desc_t": "/* 富文本表格描述。 */",
"xui_icon_category_desc_t": "/* 图标类别创建描述。 */",
"xui_icon_desc_t": "/* 图标元数据描述。 */",
"xui_icon_draw_desc_t": "/* 图标绘制描述（对齐/着色/内边距）。 */",
"xui_icon_custom_desc_t": "/* 自绘图标描述（绘制回调）。 */",
"xui_tooltip_desc_t": "/* 工具提示描述；关键字段见各行注释。 */",
"xui_widget_type_desc_t": "/* 控件类型注册描述（名称/父类型/虚函数表/实例尺寸）。 */",
}
for name, c in D.items():
    for i, l in enumerate(lines):
        if f"typedef struct {name} {{" in l:
            if not commented(i - 1):
                ins.append((i, c))
            break

FIELDS = {
"xui_button_desc_t": {"sText": "/* 按钮文本。 */", "pFont": "/* 字体；NULL 用默认。 */",
 "iTextColor": "/* 文字颜色。 */", "iDisabledTextColor": "/* 禁用态文字颜色。 */",
 "iTextFlags": "/* 文本绘制标志（对齐/省略）。 */", "iNormalColor": "/* 常态填充色。 */",
 "iHoverColor": "/* 悬停填充色。 */", "iActiveColor": "/* 按压填充色。 */",
 "iFocusColor": "/* 聚焦填充色。 */", "iDisabledColor": "/* 禁用填充色。 */",
 "iCheckedColor": "/* 选中态填充色（可选按钮）。 */", "fBorderWidth": "/* 边框宽度。 */",
 "iBorderColor": "/* 边框颜色。 */"},
"xui_label_desc_t": {"sText": "/* 文本内容。 */", "pFont": "/* 字体；NULL 用默认。 */",
 "iTextColor": "/* 文字颜色。 */", "iDisabledTextColor": "/* 禁用态文字颜色。 */",
 "iTextFlags": "/* 文本绘制标志。 */", "iWrapMode": "/* 换行模式。 */",
 "bUnderline": "/* 下划线开关。 */", "fLineGap": "/* 行距。 */", "fParagraphGap": "/* 段距。 */"},
"xui_input_desc_t": {"sText": "/* 初始文本。 */", "sPlaceholder": "/* 占位文本。 */",
 "pFont": "/* 字体；NULL 用默认。 */", "iMaxLength": "/* 最大长度；0 不限。 */",
 "iTextAlign": "/* 文本对齐。 */", "bPassword": "/* 密码模式。 */",
 "bReadonly": "/* 只读。 */", "bError": "/* 初始错误态。 */",
 "iTextColor": "/* 文字颜色。 */", "iPlaceholderColor": "/* 占位文字颜色。 */",
 "iDisabledTextColor": "/* 禁用态文字颜色。 */", "iBackgroundColor": "/* 背景色。 */",
 "iHoverBackgroundColor": "/* 悬停背景色。 */", "iDisabledBackgroundColor": "/* 禁用背景色。 */",
 "iBorderColor": "/* 边框色。 */", "iHoverBorderColor": "/* 悬停边框色。 */",
 "iFocusBorderColor": "/* 聚焦边框色。 */", "iErrorBackgroundColor": "/* 错误态背景色。 */",
 "iErrorBorderColor": "/* 错误态边框色。 */", "iSelectionColor": "/* 选区颜色。 */"},
"xui_popup_desc_t": {"pOwner": "/* 宿主控件（锚定与宽度匹配）。 */",
 "fContentWidth": "/* 内容尺寸宽。 */", "fContentHeight": "/* 内容尺寸高。 */",
 "fMaxWidth": "/* 最大宽（超出滚动）。 */", "fMaxHeight": "/* 最大高。 */",
 "fGap": "/* 与锚定间距。 */", "fOffsetX": "/* 附加偏移 X。 */", "fOffsetY": "/* 附加偏移 Y。 */",
 "fMargin": "/* 视口安全边距。 */", "fPadding": "/* 内边距。 */",
 "fBorderWidth": "/* 边框宽。 */", "fShadowSize": "/* 阴影尺寸。 */",
 "iAnchor": "/* 锚定边（八方向）。 */", "iDirection": "/* 弹出方向。 */",
 "iOutsidePolicy": "/* 外部点击关闭策略。 */", "iOwnerPolicy": "/* 宿主点击关闭策略。 */",
 "iEscapePolicy": "/* Esc 关闭策略。 */", "iFocusPolicy": "/* 焦点策略。 */",
 "bOpen": "/* 初始打开状态。 */", "bModal": "/* 模态（带护罩）。 */"},
"xui_window_desc_t": {"sTitle": "/* 标题。 */", "pFont": "/* 标题字体；NULL 用默认。 */",
 "pIconSurface": "/* 标题图标 surface。 */", "tIconSrc": "/* 图标源矩形。 */",
 "bHasIcon": "/* 是否显示图标。 */", "bClosed": "/* 初始关闭状态。 */",
 "bTopMost": "/* 置顶。 */", "bNoTitleBar": "/* 无标题栏。 */",
 "bNotMovable": "/* 禁止拖动。 */", "bDragAnywhere": "/* 任意区域拖动。 */",
 "bNotResizable": "/* 禁止缩放。 */", "bHideCollapse": "/* 隐藏折叠钮。 */",
 "bHideMaximize": "/* 隐藏最大化钮。 */", "bHideClose": "/* 隐藏关闭钮。 */",
 "bCollapsed": "/* 初始折叠。 */", "bMaximized": "/* 初始最大化。 */",
 "iResizeEdges": "/* 可缩放边位。 */", "fTitleBarHeight": "/* 标题栏高。 */",
 "fBorderWidth": "/* 边框宽。 */", "fResizeGrip": "/* 缩放握柄宽。 */"},
"xui_menu_desc_t": {"pOwner": "/* 弹出宿主控件。 */", "pFont": "/* 字体；NULL 用默认。 */",
 "tMetrics": "/* 度量组（bHasMetrics 为真时生效）。 */",
 "tColors": "/* 颜色组（bHasColors 为真时生效）。 */",
 "bHasMetrics": "/* 是否应用 tMetrics。 */", "bHasColors": "/* 是否应用 tColors。 */"},
"xui_tooltip_desc_t": {"iType": "/* 提示类型（信息/警告/错误，驱动默认色）。 */",
 "sText": "/* 提示文本。 */", "iAnchor": "/* 锚定边。 */",
 "bCustomAnchorRect": "/* 使用自定义锚定矩形。 */", "tAnchorRect": "/* 自定义锚定矩形。 */",
 "fOffsetX": "/* 偏移 X。 */", "fOffsetY": "/* 偏移 Y。 */",
 "fDelay": "/* 显示延迟（秒）。 */", "bFollowCursor": "/* 跟随光标。 */",
 "onMeasure": "/* 自定义测量回调；NULL 用默认。 */",
 "onPaint": "/* 自定义绘制回调；NULL 用默认。 */", "pUser": "/* 回调用户指针。 */"},
}
for sname, flds in FIELDS.items():
    try:
        start = next(i for i, l in enumerate(lines) if f"typedef struct {sname} {{" in l)
    except StopIteration:
        continue
    end = next(i for i in range(start, len(lines)) if lines[i].startswith("}"))
    for i in range(start + 1, end):
        m = re.search(r"\b(\w+);", lines[i])
        if m and m.group(1) in flds and not commented(i - 1):
            ins.append((i, flds[m.group(1)]))

MACRO = [("XUI_LANGUAGE_EN", "/* 内建语言 id（六种）；自定义语言从 XUI_LANGUAGE_CUSTOM_BASE 起编号。 */"),
 ("XUI_LAYER_NORMAL", "/* 覆盖层层号：普通内容 / 浮动窗 / 弹层 / 模态 / 提示 / 拖拽 / 调试。 */")]
for tok, c in MACRO:
    for i, l in enumerate(lines):
        if l.startswith("#define " + tok) and not commented(i - 1):
            ins.append((i, c)); break
for i, l in enumerate(lines):
    if re.match(r"#define\s+XUI_LAYOUT_(MANUAL|OVERLAY|ROW)", l) and not commented(i - 1):
        ins.append((i, "/* 容器布局类型常量（配合 xuiWidgetSetLayoutType）。 */")); break

seen, done = set(), 0
for idx, text in sorted(ins, key=lambda t: -t[0]):
    if idx in seen:
        continue
    seen.add(idx)
    sep_here = seps[idx] if idx < len(seps) else "\r\n"
    for tl in reversed(text.split("\n")):
        lines.insert(idx, tl)
        seps.insert(idx, sep_here or "\r\n")
    done += 1
p.write_bytes("".join(l + s for l, s in zip(lines, seps)).encode("utf-8"))
print("inserted:", done)
