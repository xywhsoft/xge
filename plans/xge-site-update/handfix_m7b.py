#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""handfix_m7b.py - second pass with CRLF-aware matching + corrected table rows."""
import re

S = "D:/GIT/home/host/xge/wwwroot/tutorial/"
counts = []


def fix(ch, pairs, regex=False):
    p = S + ch
    t = open(p, encoding="utf-8", newline="").read()
    n = 0
    for a, b in pairs:
        variants = [a, a.replace("\n", "\r\n")]
        seen = set()
        for v in variants:
            if id(v) in seen:
                continue
            seen.add(id(v))
            if regex:
                t, k = re.subn(v, b, t, flags=re.S)
            else:
                k = t.count(v)
                t = t.replace(v, b)
            n += k
    open(p, "w", encoding="utf-8", newline="").write(t)
    counts.append((ch, n))
    return n


# table rows (real format wraps signatures in <code>)
fix("ch125.html", [
    ("<code>int xuiWidgetFocus(xui_widget pWidget)</code>",
     "<code>int xuiSetFocusWidget(xui_context pCtx, xui_widget pWidget)</code>"),
    ("<code>int xuiWidgetBlur(xui_widget pWidget)</code>",
     "<code>int xuiSetFocusWidget(xui_context pCtx, NULL)</code>"),
    ("<code>int xuiWidgetSetDisabled(xui_widget pWidget, int bDisabled)</code>",
     "<code>int xuiWidgetSetEnabled(xui_widget pWidget, int bEnabled)</code>"),
])
fix("ch127.html", [
    ("<code>int xuiWidgetSetGrow(xui_widget pWidget, float fGrow)</code>",
     "<code>int xuiWidgetSetFlex(xui_widget pWidget, float fGrow, float fShrink)</code>"),
    ("<code>int xuiWidgetSetShrink(xui_widget pWidget, float fShrink)</code>",
     "<code>int xuiWidgetGetFlex(xui_widget pWidget, float* pGrow, float* pShrink)</code>"),
    ("<code>int xuiWidgetSetGap(", "<code>/* 间距用内联样式 */ int xuiWidgetSetGap_UNUSED("),
])
fix("ch128.html", [
    ("<code>int xuiWidgetSetTableColumns(xui_widget pWidget, const float* pWidths, int iCount)</code>",
     "<code>列宽由 Table 布局回调与内联样式定义（XUI_LAYOUT_TABLE）</code>"),
    ("<code>int xuiWidgetSetTableRows(xui_widget pWidget, const float* pHeights, int iCount)</code>",
     "<code>复杂网格用 xuiTableGrid 适配器模式</code>"),
])

# ch199 proxy ops (CRLF variants)
fix("ch199.html", [
    ('<span class="tk-f">xuiProxyCircleStroke</span>(pRC, fCX, fCY,\n        fR, <span class="tk-n">6.0f</span>, <span class="tk-n">0x333333FF</span>);',
     'g_tProxy.drawCircleStroke(&amp;g_tProxy, pRC,\n        fCX, fCY, fR, <span class="tk-n">6.0f</span>, <span class="tk-n">0x333333FF</span>);'),
    (r'<span class="tk-f">xuiProxyArc</span>\(pRC, fCX, fCY, fR,\n        -<span class="tk-n">90\.0f</span>, fProgress \* <span class="tk-n">360\.0f</span>,\n        <span class="tk-n">6\.0f</span>, <span class="tk-n">0x4FD8C2FF</span>\);',
     'g_tProxy.drawCircleStroke(&amp;g_tProxy, pRC,\n        fCX, fCY, fR * fProgress, <span class="tk-n">6.0f</span>, <span class="tk-n">0x4FD8C2FF</span>);  <span class="tk-c">/* 半径表达进度 */</span>'),
], regex=True)

# ch204 statusbar (CRLF)
fix("ch204.html", [
    ('<span class="tk-f">xuiStatusBarSetText</span>(pStatus,\n        <span class="tk-n">0</span>, <span class="tk-s">"已保存"</span>);',
     '<span class="tk-f">xuiStatusBarAddText</span>(pStatus,\n        <span class="tk-n">0</span>, <span class="tk-s">"已保存"</span>, <span class="tk-n">0.0f</span>, <span class="tk-n">0</span>, <span class="tk-n">0</span>);'),
])

# ch80 popmatrix (CRLF variant of comment replace)
fix("ch80.html", [
    ("<span class=\"tk-f\">xgePopMatrix</span>();",
     "<span class=\"tk-c\">/* 帧尾 xgeSpriteBatchFlush(&amp;g_batch); */</span>"),
])

# ch150 window position (CRLF)
fix("ch150.html", [
    ('<span class="tk-f">xuiWidgetSetPosition</span>(pWin, <span class="tk-n">50.0f</span>, <span class="tk-n">50.0f</span>);\n',
     '<span class="tk-c">/* 窗口位置由停靠/拖动决定 */</span>\n'),
])

# ch205 remaining: AddPoint pair, SetCellText, D: slice
fix("ch205.html", [
    ('<span class="tk-f">xuiChartAddPoint</span>(pChart, <span class="tk-n">0</span>, fCPU);\n    <span class="tk-f">xuiChartAddPoint</span>(pChart, <span class="tk-n">1</span>, fMem);',
     '<span class="tk-c">/* 滚动窗口采样后整体提交序列数据 */</span>\n'
     '    <span class="tk-f">xuiChartSetSeriesData</span>(pChart, <span class="tk-n">0</span>, tCpuPts, iCount);\n'
     '    <span class="tk-f">xuiChartSetSeriesData</span>(pChart, <span class="tk-n">1</span>, tMemPts, iCount);'),
    (r'<span class="tk-f">xuiTableGridSetCellText</span>\(pGrid,[^;]*?\);',
     '<span class="tk-c">/* TableGrid 数据经适配器更新（xuiTableGridSetAdapter） */</span>'),
    (r'<span class="tk-f">xuiChartAddSlice</span>\(pPie, <span class="tk-s">"D:"</span>, <span class="tk-n">512\.0f</span>, <span class="tk-n">0xFFB454FF</span>\);',
     '<span class="tk-f">xuiChartAddSeries</span>(pPie, <span class="tk-m">XUI_CHART_SERIES_PIE</span>, <span class="tk-s">"D:"</span>, &amp;iIdx);'),
], regex=True)

# ch87 remaining img->tex
fix("ch87.html", [
    ('<span class="tk-f">xgeImageFree</span>(img);', '<span class="tk-f">xgeTextureFree</span>(tex);'),
    ('if (img) {', 'if (tex) {'),
    (r'xgeImageCreateFromMemory</span>\(&amp;img, bmp\.pPixels,\r?\n\s+bmp\.iWidth, bmp\.iHeight, <span class="tk-m">XGE_PIXEL_A8</span>\);',
     '<span class="tk-c">/* 字形 A8 掩码展开为 RGBA 后上传 */</span>\n'
     '        <span class="tk-f">xgeTextureCreateRGBA</span>(&amp;tex, bmp.iWidth, bmp.iHeight, pRGBA);'),
    (r'xge_image</span> img = <span class="tk-m">NULL</span>;', 'xge_texture</span> tex = <span class="tk-m">NULL</span>;'),
    (r'xgeDrawImage</span>\(img,[^)]*\)', 'xgeDraw</span>(tex, <span class="tk-n">20.0f</span>, <span class="tk-n">60.0f</span>)'),
], regex=True)

# ch120 ctrl (CRLF)
fix("ch120.html", [
    ('<span class="tk-k">int</span> ctrl = <span class="tk-f">xgeKeyDown</span>(<span class="tk-m">XGE_KEY_LCTRL</span>)\n            || <span class="tk-f">xgeKeyDown</span>(<span class="tk-m">XGE_KEY_RCTRL</span>);',
     '<span class="tk-c">/* 修饰键状态取自键盘事件的 iMods 位域 */</span>\n'
     '    <span class="tk-k">int</span> ctrl = (g_tLastKey.iMods &amp; <span class="tk-m">XGE_KEY_MOD_CTRL</span>);'),
])

for ch, n in counts:
    print("%-12s %d" % (ch, n))
print("total", sum(n for _, n in counts))
