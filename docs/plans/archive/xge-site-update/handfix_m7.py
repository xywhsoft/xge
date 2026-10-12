#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""handfix_m7.py - cluster hand-fixes after apply_m7 batch stages. ASCII console."""
import re

S = "D:/GIT/home/host/xge/wwwroot/tutorial/"
counts = []


def fix(ch, pairs, regex=False):
    p = S + ch
    t = open(p, encoding="utf-8", newline="").read()
    n = 0
    for a, b in pairs:
        if regex:
            t, k = re.subn(a, b, t, flags=re.S)
        else:
            k = t.count(a)
            t = t.replace(a, b)
        n += k
    open(p, "w", encoding="utf-8", newline="").write(t)
    counts.append((ch, n))


# ---- ch10: event push comment ----
fix("ch10.html", [
    ("// xgeEventPush(...); // 推入 XGE 事件队列",
     "// xgeInputEventPost(...); // 推入 XGE 输入事件"),
])

# ---- ch69: scene translate -> matrix + DrawEx ----
fix("ch69.html", [
    ('<span class="tk-f">xgeShapeExSceneTranslate</span>(g_scroll, <span class="tk-n">0.0f</span>, -g_fOffset);',
     '<span class="tk-t">xge_shape_ex_matrix_t</span> tScroll; <span class="tk-f">xgeShapeExMatrixIdentity</span>(&amp;tScroll);\n'
     '    <span class="tk-f">xgeShapeExMatrixTranslate</span>(&amp;tScroll, <span class="tk-n">0.0f</span>, -g_fOffset);\n'
     '    <span class="tk-c">/* 绘制改用 xgeShapeExSceneDrawEx(g_scroll, 0.25f, &amp;tScroll, 1.0f) */</span>'),
])

# ---- ch80: matrix stack -> xge_draw_t ----
fix("ch80.html", [
    ('<span class="tk-f">xgePushMatrix</span>();\n    <span class="tk-f">xgeTranslate</span>(cx, cy);\n    <span class="tk-f">xgeRotate</span>(angle);\n    <span class="tk-f">xgeDrawTexture</span>(g_tex, -<span class="tk-n">64</span>, -<span class="tk-n">64</span>, <span class="tk-n">128</span>, <span class="tk-n">128</span>);',
     '<span class="tk-t">xge_draw_t</span> t = { <span class="tk-n">0</span> };\n'
     '    t.pTexture = g_tex;\n'
     '    t.tDst = (<span class="tk-t">xge_rect_t</span>){cx - <span class="tk-n">64</span>, cy - <span class="tk-n">64</span>, <span class="tk-n">128</span>, <span class="tk-n">128</span>};\n'
     '    t.tOrigin = (<span class="tk-t">xge_vec2_t</span>){<span class="tk-n">64</span>, <span class="tk-n">64</span>};  <span class="tk-c">/* 绕中心旋转 */</span>\n'
     '    t.fRotation = angle;\n'
     '    <span class="tk-f">xgeSpriteBatchAdd</span>(&amp;g_batch, &amp;t);'),
    ('<span class="tk-f">xgePopMatrix</span>();',
     '<span class="tk-c">/* 帧尾 xgeSpriteBatchFlush(&amp;g_batch); */</span>'),
])

# ---- ch81 prose ----
fix("ch81.html", [("<b>xgeDrawText</b>", "<b>xgeTextDraw</b>")])

# ---- ch87: glyph bitmap -> texture ----
fix("ch87.html", [
    ('<span class="tk-t">xge_image</span> img = <span class="tk-m">NULL</span>;',
     '<span class="tk-t">xge_texture</span> tex = <span class="tk-m">NULL</span>;'),
    (r'<span class="tk-f">xgeImageCreateFromMemory</span>\(&amp;img, bmp\.pPixels,\n            bmp\.iWidth, bmp\.iHeight, <span class="tk-m">XGE_PIXEL_A8</span>\);',
     '<span class="tk-c">/* 字形 A8 掩码展开为 RGBA 后上传 */</span>\n'
     '        <span class="tk-f">xgeTextureCreateRGBA</span>(&amp;tex, bmp.iWidth, bmp.iHeight, pRGBA);',),
    (r'<span class="tk-f">xgeDrawImage</span>\(img, <span class="tk-n">20\.0f</span>, <span class="tk-n">60\.0f</span>[^)]*\)',
     '<span class="tk-f">xgeDraw</span>(tex, <span class="tk-n">20.0f</span>, <span class="tk-n">60.0f</span>)'),
    ("<span class=\"tk-f\">xgeImageFree</span>(img);", "<span class=\"tk-f\">xgeTextureFree</span>(tex);"),
    ("if (img) {", "if (tex) {"),
], regex=True)

# ---- ch101 prose: depth flag bullet ----
fix("ch101.html", [
    ("<b>ClearFlags</b>：XGE_PASS_CLEAR_COLOR 清颜色；XGE_CLEAR_DEPTH 清深度；可组合。",
     "<b>ClearFlags</b>：XGE_PASS_CLEAR_COLOR 清颜色；2D 渲染通道没有深度清屏标志。"),
])

# ---- ch116: mouse buttons ----
fix("ch116.html", [
    ('<span class="tk-f">xgeKeyPressed</span>(<span class="tk-m">XGE_KEY_MOUSE1</span>)',
     '<span class="tk-f">xgeMouseDown</span>(<span class="tk-m">XGE_MOUSE_LEFT</span>)'),
    ('<span class="tk-f">xgeKeyReleased</span>(<span class="tk-m">XGE_KEY_MOUSE1</span>)',
     '<span class="tk-k">!</span><span class="tk-f">xgeMouseDown</span>(<span class="tk-m">XGE_MOUSE_LEFT</span>) <span class="tk-c">/* 释放边沿建议改用事件 */</span>'),
    ('<span class="tk-m">XGE_KEY_MOUSE2</span>', '<span class="tk-m">XGE_MOUSE_RIGHT</span>'),
])

# ---- ch119: gamepad axes -> indices ----
AXES = {
    "XGE_GAMEPAD_AXIS_LX": ("0", "左摇杆 X"),
    "XGE_GAMEPAD_AXIS_LY": ("1", "左摇杆 Y"),
    "XGE_GAMEPAD_AXIS_RX": ("2", "右摇杆 X"),
    "XGE_GAMEPAD_AXIS_RY": ("3", "右摇杆 Y"),
    "XGE_GAMEPAD_AXIS_RT": ("4", "右扳机"),
}
fix("ch119.html", [
    ('<span class="tk-m">%s</span>' % k,
     '<span class="tk-n">%s</span><span class="tk-c">/* %s */</span>' % (v[0], v[1]))
    for k, v in AXES.items()
])

# ---- ch120: ctrl held ----
fix("ch120.html", [
    ('<span class="tk-k">int</span> ctrl = <span class="tk-f">xgeKeyDown</span>(<span class="tk-m">XGE_KEY_LCTRL</span>)\n            || <span class="tk-f">xgeKeyDown</span>(<span class="tk-m">XGE_KEY_RCTRL</span>);',
     '<span class="tk-c">/* 修饰键状态取自键盘事件的 iMods 位域 */</span>\n'
     '    <span class="tk-k">int</span> ctrl = (g_tLastKey.iMods &amp; <span class="tk-m">XGE_KEY_MOD_CTRL</span>);'),
])

# ---- ch194: language text ids ----
fix("ch194.html", [
    ('<span class="tk-c">// 设置翻译文本</span>',
     '<span class="tk-c">// 设置翻译文本（文本槽 id 为应用自定义枚举）</span>\n'
     '<span class="tk-k">enum</span> { TXT_OK, TXT_CANCEL, TXT_YES, TXT_NO, TXT_CLOSE };'),
    ("XUI_TEXT_OK", "TXT_OK"), ("XUI_TEXT_CANCEL", "TXT_CANCEL"),
    ("XUI_TEXT_YES", "TXT_YES"), ("XUI_TEXT_NO", "TXT_NO"),
    ("XUI_TEXT_CLOSE", "TXT_CLOSE"),
])

# ---- ch197: plain-text letter keys ----
fix("ch197.html", [
    ("iKey == XGE_KEY_S &amp;&amp;", "iKey == 'S' &amp;&amp;"),
    ("iKey == XGE_KEY_Z &amp;&amp;", "iKey == 'Z' &amp;&amp;"),
])

# ---- ch125: widget state table rows ----
fix("ch125.html", [
    ("int xuiWidgetFocus(xui_widget pWidget)请求焦点",
     "int xuiSetFocusWidget(xui_context pCtx, xui_widget pWidget)请求焦点"),
    ("int xuiWidgetBlur(xui_widget pWidget)释放焦点",
     "int xuiSetFocusWidget(xui_context pCtx, NULL)释放焦点"),
    ("int xuiWidgetSetDisabled(xui_widget pWidget, int bDisabled)设置禁用",
     "int xuiWidgetSetEnabled(xui_widget pWidget, int bEnabled)启用/禁用"),
])

# ---- ch127: flex rows ----
fix("ch127.html", [
    ("int xuiWidgetSetGrow(xui_widget pWidget, float fGrow)设置伸展因子",
     "int xuiWidgetSetFlex(xui_widget pWidget, float fGrow, float fShrink)伸展/收缩因子"),
    ("int xuiWidgetSetShrink(xui_widget pWidget, float fShrink)设置收缩因子",
     "int xuiWidgetGetFlex(xui_widget pWidget, float* pGrow, float* pShrink)读取因子"),
])

# ---- ch128: table rows ----
fix("ch128.html", [
    ("int xuiWidgetSetTableColumns(xui_widget pWidget, const float* pWidths, int iCount)定义列宽",
     "Table 布局的列宽由布局回调与内联样式定义（XUI_LAYOUT_TABLE）"),
    ("int xuiWidgetSetTableRows(xui_widget pWidget, const float* pHeights, int iCount)定义行高",
     "行高同理；复杂网格用 xuiTableGrid 的适配器模式"),
])

# ---- ch130: event rows ----
fix("ch130.html", [
    ("int xuiWidgetSetEventMask(xui_widget pW, uint64_t iMask)设置事件掩码",
     "uint64_t xuiWidgetGetEventMask(xui_widget pW)读取事件掩码"),
    ("int xuiEventStopPropagation(xui_event* pEvent)阻止冒泡",
     "回调返回 XUI_EVENT_DISPATCH_STOP 阻止传播"),
    ("int xuiEventPreventDefault(xui_event* pEvent)阻止默认行为",
     "默认行为由具体控件的命令处理决定"),
    ("xui_event*", "xui_event_t*"),
])

# ---- ch141: surface via proxy ----
fix("ch141.html", [
    ('<span class="tk-f">xuiSurfaceCreate</span>(g_pCtx, &amp;g_pSurf, <span class="tk-n">64</span>, <span class="tk-n">64</span>, pixels);',
     '<span class="tk-t">xui_surface_desc_t</span> tSD = { <span class="tk-n">0</span> };\n'
     '    tSD.iWidth = tSD.iHeight = <span class="tk-n">64</span>; tSD.iFormat = XUI_SURFACE_FORMAT_RGBA8;\n'
     '    tXgeProxy.surfaceCreate(&amp;tXgeProxy, &amp;g_pSurf, &amp;tSD);'),
])

# ---- ch150: manual layout + window position ----
fix("ch150.html", [
    ('<span class="tk-f">xuiManualCreate</span>(g_pCtx, &amp;pRoot, <span class="tk-m">NULL</span>);',
     '<span class="tk-f">xuiWidgetCreate</span>(g_pCtx, &amp;pRoot);\n'
     '    <span class="tk-f">xuiWidgetSetLayoutType</span>(pRoot, <span class="tk-m">XUI_LAYOUT_MANUAL</span>);'),
    ('<span class="tk-f">xuiWidgetSetPosition</span>(pWin, <span class="tk-n">50.0f</span>, <span class="tk-n">50.0f</span>);\n',
     '<span class="tk-c">/* 窗口位置由停靠/拖动决定 */</span>\n'),
])

# ---- ch189: qr position ----
fix("ch189.html", [
    ('    <span class="tk-f">xuiWidgetSetPosition</span>(pQR, <span class="tk-n">50</span>, <span class="tk-n">50</span>);',
     '    <span class="tk-c">/* 位置已含于上方 SetRect */</span>'),
])

# ---- ch192: normal state ----
fix("ch192.html", [
    ("XUI_STATE_NORMAL", "0 /* 默认态 */"),
])

# ---- ch198: hittest flags ----
fix("ch198.html", [
    ("XUI_HITTEST_INCLUDE_DISABLED", "0"),
])

# ---- ch199: proxy ops + create + pos ----
fix("ch199.html", [
    ('<span class="tk-f">xuiProxyCircleStroke</span>(pRC, fCX, fCY,\n        fR, <span class="tk-n">6.0f</span>, <span class="tk-n">0x333333FF</span>);',
     'g_tProxy.drawCircleStroke(&amp;g_tProxy, pRC,\n        fCX, fCY, fR, <span class="tk-n">6.0f</span>, <span class="tk-n">0x333333FF</span>);'),
    (r'<span class="tk-f">xuiProxyArc</span>\(pRC, fCX, fCY, fR,\n        -<span class="tk-n">90\.0f</span>, fProgress \* <span class="tk-n">360\.0f</span>,\n        <span class="tk-n">6\.0f</span>, <span class="tk-n">0x4FD8C2FF</span>\);',
     'g_tProxy.drawCircleStroke(&amp;g_tProxy, pRC,\n        fCX, fCY, fR * fProgress, <span class="tk-n">6.0f</span>, <span class="tk-n">0x4FD8C2FF</span>);  <span class="tk-c">/* 半径表达进度 */</span>'),
    ('<span class="tk-t">xui_widget</span> pRing = <span class="tk-f">xuiWidgetCreate</span>(g_pCtx, pRoot, pType);',
     '<span class="tk-t">xui_widget</span> pRing;\n'
     '    <span class="tk-f">xuiWidgetCreate</span>(g_pCtx, &amp;pRing);\n'
     '    <span class="tk-f">xuiWidgetAddChild</span>(pRoot, pRing);'),
    ('<span class="tk-f">xuiWidgetSetPos</span>(pRing, <span class="tk-n">270</span>, <span class="tk-n">180</span>);',
     '<span class="tk-f">xuiWidgetSetRect</span>(pRing, (<span class="tk-t">xui_rect_t</span>){<span class="tk-n">270</span>, <span class="tk-n">180</span>, <span class="tk-n">120</span>, <span class="tk-n">120</span>});'),
], regex=True)

# ---- ch201: caps + fields + context ----
fix("ch201.html", [
    ("tProxy.iCaps = XUI_PROXY_CAP_RECT |\n               XUI_PROXY_CAP_CIRCLE |\n               XUI_PROXY_CAP_SURFACE |\n               XUI_PROXY_CAP_CLIP;",
     "tProxy.iCaps = XUI_PROXY_CAP_SURFACE_TARGET |\n               XUI_PROXY_CAP_SHAPE |\n               XUI_PROXY_CAP_PATH_FILL |\n               XUI_PROXY_CAP_PATH_STROKE;"),
    ("tProxy.onDrawRect = ProxyDrawRect;\ntProxy.onClipBegin = ProxyClipBegin;",
     "tProxy.drawRectFill = ProxyDrawRect;  <span class=\"tk-c\">/* 其余 ops 按 xui_proxy_t 字段填充 */</span>"),
    ('<span class="tk-f">xuiContextCreate</span>(&amp;pCtx, &amp;tProxy, <span class="tk-k">NULL</span>);',
     '<span class="tk-f">xuiCreate</span>(&amp;pCtx);\n<span class="tk-f">xuiSetProxy</span>(pCtx, &amp;tProxy);'),
])

# ---- ch202: context loop + modal ----
fix("ch202.html", [
    ('<span class="tk-f">xuiContextUpdate</span>(pCtx, fDT);',
     '<span class="tk-f">xuiUpdate</span>(pCtx, fDT);'),
    ('<span class="tk-f">xuiContextRender</span>(pCtx);',
     '<span class="tk-f">xuiLayout</span>(pCtx);  <span class="tk-c">/* 绘制由根控件经代理完成 */</span>'),
    ('<span class="tk-f">xuiContextHasModal</span>(pCtx)',
     'ModalActive() <span class="tk-c">/* 用 xuiPopupIsModal/xuiMsgBoxIsModal 按实例查询 */</span>'),
])

# ---- ch204: statusbar ----
fix("ch204.html", [
    ('<span class="tk-f">xuiStatusBarSetText</span>(pStatus,\n        <span class="tk-n">0</span>, <span class="tk-s">"已保存"</span>);',
     '<span class="tk-f">xuiStatusBarAddText</span>(pStatus,\n        <span class="tk-n">0</span>, <span class="tk-s">"已保存"</span>, <span class="tk-n">0.0f</span>, <span class="tk-n">0</span>, <span class="tk-n">0</span>);'),
])

# ---- ch205: chart + grid ----
fix("ch205.html", [
    ('<span class="tk-f">xuiChartAddPoint</span>(pChart, <span class="tk-n">0</span>, fCPU);\n    <span class="tk-f">xuiChartAddPoint</span>(pChart, <span class="tk-n">1</span>, fMem);',
     '<span class="tk-c">/* 滚动窗口采样后整体提交序列数据 */</span>\n'
     '    <span class="tk-f">xuiChartSetSeriesData</span>(pChart, <span class="tk-n">0</span>, tCpuPts, iCount);\n'
     '    <span class="tk-f">xuiChartSetSeriesData</span>(pChart, <span class="tk-n">1</span>, tMemPts, iCount);'),
    (r'<span class="tk-f">xuiChartAddSlice</span>\(pPie, <span class="tk-s">"C:"</span>[^)]*\)',
     '<span class="tk-f">xuiChartAddSeries</span>(pPie, <span class="tk-m">XUI_CHART_SERIES_PIE</span>, <span class="tk-s">"C:"</span>, &amp;iIdx);'),
    (r'<span class="tk-f">xuiTableGridSetCellText</span>\(pGrid,\n        g_iSelRow, COL_CPU, [^)]*\)',
     '<span class="tk-c">/* TableGrid 数据经适配器更新（xuiTableGridSetAdapter） */</span>'),
], regex=True)

# ---- ch209: touch + dpi ----
fix("ch209.html", [
    ('<span class="tk-t">xge_touch_t</span> tTouch = <span class="tk-f">xgeTouchGet</span>(i);',
     '<span class="tk-t">xge_touch_point_t</span> tTouch;\n'
     '        <span class="tk-f">xgeTouchGet</span>(i, &amp;tTouch);'),
    ("tTouch.bBegan", "tTouch.iPhase == XGE_TOUCH_BEGIN"),
    ("tTouch.bEnded", "tTouch.iPhase == XGE_TOUCH_END"),
    ('<span class="tk-f">xuiContextSetScale</span>(pCtx, fDPI);',
     '<span class="tk-f">xuiSetVirtualDpi</span>(pCtx, fDPI);'),
])

# ---- leftover SetPos -> SetRect (sizes illustrative) ----
fix("ch166.html", [
    ('<span class="tk-f">xuiWidgetSetPos</span>(pPicker, <span class="tk-n">300</span>, <span class="tk-n">150</span>);',
     '<span class="tk-f">xuiWidgetSetRect</span>(pPicker, (<span class="tk-t">xui_rect_t</span>){<span class="tk-n">300</span>, <span class="tk-n">150</span>, <span class="tk-n">220</span>, <span class="tk-n">220</span>});'),
])
fix("ch196.html", [
    ('<span class="tk-f">xuiWidgetSetPos</span>(g_pGrid, <span class="tk-n">160</span>, <span class="tk-n">140</span>);',
     '<span class="tk-f">xuiWidgetSetRect</span>(g_pGrid, (<span class="tk-t">xui_rect_t</span>){<span class="tk-n">160</span>, <span class="tk-n">140</span>, <span class="tk-n">480</span>, <span class="tk-n">320</span>});'),
])
fix("ch206.html", [
    ('<span class="tk-f">xuiWidgetSetPos</span>(pStick, <span class="tk-n">40</span>, <span class="tk-n">360</span>);',
     '<span class="tk-f">xuiWidgetSetRect</span>(pStick, (<span class="tk-t">xui_rect_t</span>){<span class="tk-n">40</span>, <span class="tk-n">360</span>, <span class="tk-n">120</span>, <span class="tk-n">120</span>});'),
])
fix("ch200.html", [
    (r'<span class="tk-f">xuiWidgetSetPos</span>\(pB, ([^;]+)\);',
     r'<span class="tk-f">xuiWidgetSetRect</span>(pB, (<span class="tk-t">xui_rect_t</span>){\1, <span class="tk-n">48</span>, <span class="tk-n">48</span>});'),
], regex=True)

# ---- ch203: my earlier B2 wrong name ----
fix("ch203.html", [("xuiTimelineViewCreate", "xuiTimeLineViewCreate")])

# global: bare xui_event -> xui_event_t in remaining prose
fix("ch130.html", [(r"\bxui_event\b", "xui_event_t")], regex=True)

for ch, n in counts:
    if n:
        print("%-12s %d replaces" % (ch, n))
print("total", sum(n for _, n in counts))
