#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""handfix_m7c.py - third pass: CRLF-regex for remaining sites."""
import re

S = "D:/GIT/home/host/xge/wwwroot/tutorial/"
counts = []


def fix(ch, pairs):
    p = S + ch
    t = open(p, encoding="utf-8", newline="").read()
    n = 0
    for a, b in pairs:
        t, k = re.subn(a, b, t)
        n += k
    open(p, "w", encoding="utf-8", newline="").write(t)
    counts.append((ch, n))


def raw(s):
    return s.replace(r"\n", r"\r?\n").replace("\n", r"\r?\n")


R = '<span class="tk-f">%s</span>'

fix("ch80.html", [
    (raw(r'    <span class="tk-f">xgePushMatrix</span>\(\);\n    <span class="tk-f">xgeTranslate</span>\(cx, cy\);\n    <span class="tk-f">xgeRotate</span>\(angle\);\n    <span class="tk-f">xgeDrawTexture</span>\(g_tex, -<span class="tk-n">64</span>, -<span class="tk-n">64</span>, <span class="tk-n">128</span>, <span class="tk-n">128</span>\);'),
     '<span class="tk-t">xge_draw_t</span> t = { <span class="tk-n">0</span> };\n'
     '    t.pTexture = g_tex;\n'
     '    t.tDst = (<span class="tk-t">xge_rect_t</span>){cx - <span class="tk-n">64</span>, cy - <span class="tk-n">64</span>, <span class="tk-n">128</span>, <span class="tk-n">128</span>};\n'
     '    t.tOrigin = (<span class="tk-t">xge_vec2_t</span>){<span class="tk-n">64</span>, <span class="tk-n">64</span>};  <span class="tk-c">/* 绕中心旋转 */</span>\n'
     '    t.fRotation = angle;\n'
     '    <span class="tk-f">xgeSpriteBatchAdd</span>(&amp;g_batch, &amp;t);'),
])

fix("ch191.html", [
    (r'<span class="tk-f">xgeMousePressed</span>\(XGE_MOUSE_LEFT\)',
     '<span class="tk-f">xgeMouseDown</span>(<span class="tk-m">XGE_MOUSE_LEFT</span>)'),
    (raw(r'ev\.fX = <span class="tk-f">xgeMouseGetX</span>\(\);\n\s*ev\.fY = <span class="tk-f">xgeMouseGetY</span>\(\);'),
     '<span class="tk-k">float</span> mx, my;\n'
     '        <span class="tk-f">xgeMouseGet</span>(&amp;mx, &amp;my);\n'
     '        ev.fX = mx; ev.fY = my;'),
])

fix("ch199.html", [
    (raw(r'<span class="tk-f">xuiProxyCircleStroke</span>\(pRC, fCX, fCY,\n\s*fR, <span class="tk-n">6\.0f</span>, <span class="tk-n">0x333333FF</span>\);'),
     'g_tProxy.drawCircleStroke(&amp;g_tProxy, pRC,\n        fCX, fCY, fR, <span class="tk-n">6.0f</span>, <span class="tk-n">0x333333FF</span>);'),
    (raw(r'<span class="tk-f">xuiProxyCircleStroke</span>\(pRC, cx, cy, rad, <span class="tk-n">6\.0f</span>, <span class="tk-n">0x333333FF</span>\);'),
     'g_tProxy.drawCircleStroke(&amp;g_tProxy, pRC, cx, cy, rad, <span class="tk-n">6.0f</span>, <span class="tk-n">0x333333FF</span>);'),
    (raw(r'<span class="tk-f">xuiProxyArc</span>\(pRC, fCX, fCY, fR,\n\s*-<span class="tk-n">90\.0f</span>, fProgress \* <span class="tk-n">360\.0f</span>,\n\s*<span class="tk-n">6\.0f</span>, <span class="tk-n">0x4FD8C2FF</span>\);'),
     'g_tProxy.drawCircleStroke(&amp;g_tProxy, pRC,\n        fCX, fCY, fR * fProgress, <span class="tk-n">6.0f</span>, <span class="tk-n">0x4FD8C2FF</span>);  <span class="tk-c">/* 半径表达进度 */</span>'),
    (raw(r'<span class="tk-f">xuiProxyArc</span>\(pRC, cx, cy, rad,\n\s*-<span class="tk-n">90\.0f</span>[^;]*?\);'),
     'g_tProxy.drawCircleStroke(&amp;g_tProxy, pRC,\n        cx, cy, rad * fProgress, <span class="tk-n">6.0f</span>, <span class="tk-n">0x4FD8C2FF</span>);  <span class="tk-c">/* 半径表达进度 */</span>'),
    (raw(r'<span class="tk-f">xuiWidgetSetPos</span>\(pRing, <span class="tk-n">270</span>, <span class="tk-n">180</span>\);'),
     '<span class="tk-f">xuiWidgetSetRect</span>(pRing, (<span class="tk-t">xui_rect_t</span>){<span class="tk-n">270</span>, <span class="tk-n">180</span>, <span class="tk-n">120</span>, <span class="tk-n">120</span>});'),
])

fix("ch201.html", [
    (raw(r'tProxy\.iCaps = XUI_PROXY_CAP_RECT \|\n\s*XUI_PROXY_CAP_CIRCLE \|\n\s*XUI_PROXY_CAP_SURFACE \|\n\s*XUI_PROXY_CAP_CLIP;'),
     'tProxy.iCaps = XUI_PROXY_CAP_SURFACE_TARGET |\n'
     '               XUI_PROXY_CAP_SHAPE |\n'
     '               XUI_PROXY_CAP_PATH_FILL |\n'
     '               XUI_PROXY_CAP_PATH_STROKE;'),
    (raw(r'tProxy\.onDrawRect = ProxyDrawRect;\ntProxy\.onClipBegin = ProxyClipBegin;'),
     'tProxy.drawRectFill = ProxyDrawRect;  <span class="tk-c">/* 其余 ops 按 xui_proxy_t 字段填充 */</span>'),
])

fix("ch141.html", [
    (r'<b>xuiSurfaceCreate</b>：将像素数据上传为 GPU 纹理供 Image 控件使用。',
     '<b>代理 surfaceCreate</b>：经渲染代理将像素数据上传为 GPU 纹理供 Image 控件使用。'),
])

fix("ch205.html", [
    (raw(r'<span class="tk-f">xuiChartAddPoint</span>\(pChart, <span class="tk-n">0</span>, fCPU\);\n\s*<span class="tk-f">xuiChartAddPoint</span>\(pChart, <span class="tk-n">1</span>, fMem\);'),
     '<span class="tk-c">/* 滚动窗口采样后整体提交序列数据 */</span>\n'
     '    <span class="tk-f">xuiChartSetSeriesData</span>(pChart, <span class="tk-n">0</span>, tCpuPts, iCount);\n'
     '    <span class="tk-f">xuiChartSetSeriesData</span>(pChart, <span class="tk-n">1</span>, tMemPts, iCount);'),
])

fix("ch130.html", [
    (r'<code>int xuiEventStopPropagation\(xui_event_t\* pEvent\)</code></td><td>阻止冒泡',
     '<code>回调返回 XUI_EVENT_DISPATCH_STOP</code></td><td>阻止传播'),
    (r'<code>int xuiEventPreventDefault\(xui_event_t\* pEvent\)</code></td><td>阻止默认行为',
     '<code>默认行为</code></td><td>由具体控件的命令处理决定'),
    (raw(r'<span class="tk-f">xuiEventStopPropagation</span>\(pEvent\);'),
     '<span class="tk-k">return</span> <span class="tk-m">XUI_EVENT_DISPATCH_STOP</span>;'),
    (raw(r'<span class="tk-f">xuiWidgetSetEventMask</span>\([^)]*\);'),
     '<span class="tk-c">/* 掩码可读（xuiWidgetGetEventMask），过滤在回调内判断 */</span>'),
])

fix("ch197.html", [
    (r'<span class="tk-f">xgeKeyDown</span>\(<span class="tk-m">XGE_KEY_LCTRL</span>\)',
     '(iMods &amp; <span class="tk-m">XGE_KEY_MOD_CTRL</span>)'),
])

fix("ch125.html", [
    (r'<span class="tk-f">xuiWidgetFocus</span>\((\w+)\);',
     r'<span class="tk-f">xuiSetFocusWidget</span>(g_pCtx, \1);'),
])

fix("ch127.html", [
    (r'<span class="tk-f">xuiWidgetSetGrow</span>\((\w+), ([^,)]+)\);',
     r'<span class="tk-f">xuiWidgetSetFlex</span>(\1, \2, <span class="tk-n">1.0f</span>);'),
    (r'<tr><td><code>/\* 间距用内联样式 \*/ int xuiWidgetSetGap_UNUSED\([^<]*\)</code></td><td>[^<]*</td></tr>\r?\n?',
     ''),
    (r'<span class="tk-f">xuiWidgetSetLayout</span>\(',
     r'<span class="tk-f">xuiWidgetSetLayoutType</span>('),
])

fix("ch128.html", [
    (raw(r'<span class="tk-f">xuiWidgetSetTableColumns</span>\(pForm, cols, <span class="tk-n">2</span>\);'),
     '<span class="tk-c">/* Table 列宽由布局回调/内联样式定义 */</span>'),
    (r'<span class="tk-f">xuiWidgetSetTableCell</span>\(',
     r'<span class="tk-f">xuiWidgetSetRect</span>('),
])

fix("ch206.html", [
    (r'<span class="tk-f">xuiWidgetSetPos</span>\(pBar, <span class="tk-n">250</span>, <span class="tk-n">440</span>\);',
     '<span class="tk-f">xuiWidgetSetRect</span>(pBar, (<span class="tk-t">xui_rect_t</span>){<span class="tk-n">250</span>, <span class="tk-n">440</span>, <span class="tk-n">480</span>, <span class="tk-n">120</span>});'),
])

fix("ch90.html", [
    (r'<span class="tk-m">XGE_TEXT_WRAP</span>', r'<span class="tk-m">XGE_TEXT_CLIP</span>'),
    ("按宽度自动换行", "超出宽度裁剪"),
])

for ch, n in counts:
    print("%-12s %d" % (ch, n))
print("total", sum(n for _, n in counts))
