#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""apply_m7.py - legacy tutorial API-name remediation, stages A (token renames)
and B (template-structure transforms). Idempotent. ASCII console output.
Manual rewrites for remaining clusters are done afterwards via editor.
"""
import glob
import os
import re
import sys

SITE = "D:/GIT/home/host/xge/wwwroot/tutorial"
stats = {}


def rep(text, pattern, repl, tag, flags=0):
    new, n = re.subn(pattern, repl, text, flags=flags)
    if n:
        stats[tag] = stats.get(tag, 0) + n
    return new


def split_args(blob):
    """Split a captured HTML arg-blob on top-level ', ' (quote/tag aware)."""
    args, depth, cur, i, inq = [], 0, "", 0, False
    while i < len(blob):
        c = blob[i]
        if c == '"':
            inq = not inq
        if not inq:
            if c == '<':
                depth += 1
            elif c == '>':
                depth -= 1
            elif c == ',' and depth == 0:
                args.append(cur.strip())
                cur = ""
                i += 1
                if blob[i:i + 1] == ' ':
                    i += 1
                continue
        cur += c
        i += 1
    if cur.strip():
        args.append(cur.strip())
    return args


WIDGET_CREATE = {
    "PANEL": "xuiPanelCreate", "LABEL": "xuiLabelCreate", "BUTTON": "xuiButtonCreate",
    "CANVAS": "xuiCanvasCreate", "LISTVIEW": "xuiListViewCreate",
    "TREEVIEW": "xuiTreeViewCreate", "TABS": "xuiTabsCreate",
    "TIMELINEVIEW": "xuiTimeLineViewCreate",
}


def transform(fname):
    p = os.path.join(SITE, fname)
    with open(p, encoding="utf-8", newline="") as f:
        t = f.read()
    orig = t

    # ---------- Stage A: plain token renames ----------
    renames = [
        (r"\bxgeGetDeltaTime\b", "xgeGetDelta"),
        (r"\bxgeInputKeyDown\b", "xgeKeyDown"),
        (r"\bxgeKeyHit\b", "xgeKeyPressed"),
        (r"\bxgeKeyIsDown\b", "xgeKeyDown"),
        (r"\bxgeMouseGetPos\b", "xgeMouseGet"),
        (r"\bxgeMousePos\b", "xgeMouseGet"),
        (r"\bXGE_BLEND_ADDITIVE\b", "XGE_BLEND_ADD"),
        (r"\bXGE_BLEND_NORMAL\b", "XGE_BLEND_ALPHA"),
        (r"\bXGE_CLEAR_COLOR\b", "XGE_PASS_CLEAR_COLOR"),
        (r"\bXGE_FMT_R8\b", "XGE_PIXEL_A8"),
        (r"\bXGE_GAMEPAD_BTN_A\b", "XGE_GAMEPAD_A"),
        (r"\bXGE_PAD_DPAD_RIGHT\b", "XGE_GAMEPAD_DPAD_RIGHT"),
        (r"\bXGE_KEY_LSHIFT\b", "XGE_KEY_LEFT_SHIFT"),
        (r"\bXGE_MOD_CTRL\b", "XGE_KEY_MOD_CTRL"),
        (r"\bXGE_TEXT_DECO_UNDERLINE\b", "XGE_TEXT_DECORATION_UNDERLINE"),
        (r"\bXGE_TEXT_DECO_STRIKETHROUGH\b", "XGE_TEXT_DECORATION_STRIKE"),
        (r"\bXGE_TOUCH_MOVED\b", "XGE_TOUCH_MOVE"),
        (r"\bXGE_USAGE_DYNAMIC\b", "XGE_BUFFER_DYNAMIC"),
        (r"\bXUI_CHART_LINE\b", "XUI_CHART_SERIES_LINE"),
        (r"\bXUI_CHART_PIE\b", "XUI_CHART_SERIES_PIE"),
        (r"\bXUI_DOCK_CENTER\b", "XUI_DOCK_FILL"),
        (r"\bXUI_EVENT_MOUSE_DOWN\b", "XUI_EVENT_POINTER_DOWN"),
        (r"\bXUI_EVENT_TOUCH_DOWN\b", "XUI_EVENT_POINTER_DOWN"),
        (r"\bXUI_EVENT_TOUCH_MOVE\b", "XUI_EVENT_POINTER_MOVE"),
        (r"\bXUI_EVENT_TOUCH_UP\b", "XUI_EVENT_POINTER_UP"),
        (r"\bXUI_PHASE_CAPTURE\b", "XUI_EVENT_PHASE_CAPTURE"),
        (r"\bXUI_STATE_HOVER\b", "XUI_WIDGET_STATE_HOVER"),
        (r"\bXUI_STATE_ACTIVE\b", "XUI_WIDGET_STATE_ACTIVE"),
        (r"\bxgeDrawCircle\b", "xgeShapeCircleFill"),
        (r"\bxgeShapeCircle\b", "xgeShapeCircleFill"),
        (r"\bxgeShapeEllipse\b", "xgeShapeEllipseFill"),
        (r"\bxgeShapeSectorFill\b", "xgeShapePieFill"),
        (r"\bxgeShapeExSceneAddShape\b", "xgeShapeExSceneAdd"),
        (r"\bxuiTimelineView\b", "xuiTimeLineView"),
        (r"\bxgeImageDestroy\b", "xgeImageFree"),
        (r"\bxgeTextureDestroy\b", "xgeTextureFree"),
        (r"\bxuiWidgetAddClass\b", "xuiWidgetAddStyleClass"),
        (r"\bxuiWidgetGetState\b", "xuiWidgetGetInputState"),
        (r"\bxuiCodeEditSetLexer\b", "xuiCodeEditSetLanguage"),
        (r"\bxui_event_proc\b", "xui_widget_event_proc"),
        (r"\bxui_render_context\b", "xui_draw_context"),
        (r"\bxui_menu_item_desc_t\b", "xui_menu_item_t"),
        (r"\bxge_matrix_t\b", "xge_mat3_t"),
        (r"\bxuiTabsGetCurrentPage\b", "xuiTabsGetClientWidget"),
        (r"\bxuiScrollViewScrollToBottom\b", "xuiScrollViewEnsureChildVisible"),
    ]
    for pat, name in renames:
        t = rep(t, pat, lambda m, n=name: n, "A:" + name)

    # XGE_CLEAR_DEPTH removal: " | XGE_CLEAR_DEPTH" dropped (2D passes have no depth flag)
    t = rep(t, r' \| <span class="tk-m">XGE_CLEAR_DEPTH</span>', "", "A:drop-depth")

    # XGE_PI -> literal
    t = rep(t, r"\bXGE_PI\b", "3.14159265f", "A:XGE_PI->lit")

    # single-char key macros -> char literals
    t = rep(t, r'<span class="tk-m">XGE_KEY_([A-Z0-9])</span>', r"<span class=\"tk-s\">'\1'</span>",
            "A:key-literal")

    # XUI_STATE_NORMAL -> cache state id 0 (default)
    t = rep(t, r'<span class="tk-m">XUI_STATE_NORMAL</span>', r'<span class="tk-n">0</span>', "A:state-normal")

    # ---------- Stage B ----------
    # B1 proxy idiom
    t = rep(t,
            r'<span class="tk-f">xuiSetProxy</span>\((\w+), &amp;xge_xui_proxy\);',
            r'<span class="tk-t">xui_proxy_t</span> tXgeProxy = <span class="tk-f">xuiProxyXge</span>();\n<span class="tk-f">xuiSetProxy</span>(\1, &amp;tXgeProxy);',
            "B1:proxy-idiom")
    t = rep(t,
            r'<span class="tk-k">extern const</span> <span class="tk-t">xui_proxy_t</span> xge_xui_proxy;',
            r'<span class="tk-c">/* XGE 官方代理由 xuiProxyXge() 提供 */</span>',
            "B1:proxy-decl")
    t = rep(t, r"\bxge_xui_proxy\b", "tXgeProxy", "B1:proxy-token")

    # B2 typed widget create -> per-widget create (+AddChild when parented)
    def b2(m):
        var, ctx, parent, whole, kind = m.group(1), m.group(2), m.group(3), m.group(4), m.group(5)
        create = WIDGET_CREATE.get(kind)
        if not create:
            return m.group(0)
        out = ('<span class="tk-t">xui_widget</span> %s;\n<span class="tk-f">%s</span>(%s, &amp;%s, '
               '<span class="tk-k">NULL</span>);') % (var, create, ctx, var)
        if 'NULL' not in parent:
            out += '\n<span class="tk-f">xuiWidgetAddChild</span>(%s, %s);' % (parent, var)
        stats["B2:create-" + create] = stats.get("B2:create-" + create, 0) + 1
        return out
    t = rep(t,
            r'<span class="tk-t">xui_widget</span> (\w+) = <span class="tk-f">xuiWidgetCreate</span>\((\w+), ([^,)]+), <span class="tk-m">(XUI_WIDGET_(\w+))</span>\);',
            b2, "B2:create")

    # B2b: assignment without the type-span prefix (globals etc.)
    def b2b(m):
        var, ctx, parent, kind = m.group(1), m.group(2), m.group(3), m.group(4)
        create = WIDGET_CREATE.get(kind)
        if not create:
            return m.group(0)
        out = '%s = <span class="tk-f">%s</span>(%s, &amp;%s, <span class="tk-k">NULL</span>);' % (var, create, ctx, var)
        if 'NULL' not in parent:
            out += '\n<span class="tk-f">xuiWidgetAddChild</span>(%s, %s);' % (parent, var)
        stats["B2b:create-" + create] = stats.get("B2b:create-" + create, 0) + 1
        return out
    t = rep(t,
            r'(\w+) = <span class="tk-f">xuiWidgetCreate</span>\((\w+), ([^,)]+), <span class="tk-m">XUI_WIDGET_(\w+)</span>\);',
            b2b, "B2b:create")

    # generic 3-arg create without type constant (ch199 custom widget) - flag for hand fix
    def b2g(m):
        stats["B2:generic-HANDFIX"] = stats.get("B2:generic-HANDFIX", 0) + 1
        return m.group(0)
    t = rep(t,
            r'<span class="tk-f">xuiWidgetCreate</span>\((\w+), ([^,)&]+), ([^,)&]+)\);',
            b2g, "B2:create-generic")

    # B3 event callback registration drops the event-type arg
    t = rep(t,
            r'<span class="tk-f">xuiWidgetSetCallback</span>\((\w+), <span class="tk-m">XUI_EVENT_CLICK</span>, ',
            r'<span class="tk-f">xuiWidgetSetEventCallback</span>(\1, ',
            "B3:callback")
    t = rep(t, r"\bXUI_EVENT_CLICK\b", "XUI_EVENT_POINTER_CLICK", "A:EVT_CLICK")

    # B4 SetText per widget kind (collect kinds from creates in this file)
    kinds = dict(re.findall(r'<span class="tk-f">xui(Label|Button)Create</span>\(\w+, &amp;(\w+)', t))
    def b4(m):
        var = m.group(1)
        fn = "xuiButtonSetText" if kinds.get(var) == "Button" else "xuiLabelSetText"
        stats["B4:" + fn] = stats.get("B4:" + fn, 0) + 1
        return '<span class="tk-f">%s</span>(%s, ' % (fn, var)
    t = rep(t, r'<span class="tk-f">xuiWidgetSetText</span>\((\w+), ', b4, "B4:settext")

    # B5 SetSize/SetPos -> SetRect (per code block, stateful)
    def b5_block(block):
        sizes = {}
        def grab(m):
            sizes[m.group(1)] = (m.group(2), m.group(3))
            return "\x00SIZE\x00" + m.group(1) + "\x00"
        block2 = re.sub(r'<span class="tk-f">xuiWidgetSetSize</span>\((\w+), ([^,)]+), ([^,)]+)\);', grab, block)
        def posrep(m):
            var, x, y = m.group(1), m.group(2), m.group(3)
            wh = sizes.get(var)
            if not wh:
                return m.group(0)
            stats["B5:rect"] = stats.get("B5:rect", 0) + 1
            return ('<span class="tk-f">xuiWidgetSetRect</span>(%s, (<span class="tk-t">xui_rect_t</span>){%s, %s, %s, %s});'
                    % (var, x, y, wh[0], wh[1]))
        block2 = re.sub(r'<span class="tk-f">xuiWidgetSetPos</span>\((\w+), ([^,)]+), ([^,)]+)\);', posrep, block2)
        def sizerep(m):
            var = m.group(1)
            wh = sizes.get(var)
            stats["B5:rect-sizeonly"] = stats.get("B5:rect-sizeonly", 0) + 1
            return ('<span class="tk-f">xuiWidgetSetRect</span>(%s, (<span class="tk-t">xui_rect_t</span>){0, 0, %s, %s});'
                    % (var, wh[0], wh[1]))
        block2 = re.sub(r'\x00SIZE\x00(\w+)\x00', sizerep, block2)
        return block2
    parts = re.split(r'(<pre>.*?</pre>)', t, flags=re.S)
    t = "".join(b5_block(p) if p.startswith("<pre>") else p for p in parts)

    # B6 ColumnCreate -> WidgetCreate + layout type
    t = rep(t,
            r'<span class="tk-f">xuiColumnCreate</span>\((\w+), &amp;(\w+), ([^,)]+)\);',
            r'<span class="tk-f">xuiWidgetCreate</span>(\1, &amp;\2);\n<span class="tk-f">xuiWidgetSetLayoutType</span>(\2, <span class="tk-m">XUI_LAYOUT_COLUMN</span>);',
            "B6:column")
    t = rep(t,
            r'<span class="tk-f">xuiColumnSetGap</span>\(\w+, [^,)]+\);',
            '<span class="tk-c">/* 间距：用内联样式或子控件布局参数设置 */</span>',
            "B6:gap")

    # B8 xgeDrawText arg order -> xgeTextDraw(font, text, x, y, color)
    def b8(m):
        args = split_args(m.group(2))
        if len(args) != 5:
            stats["B8:SKIP-argc"] = stats.get("B8:SKIP-argc", 0) + 1
            return m.group(0)
        a_font, a1, a2, a3, a_col = args
        if 'tk-s' in a3 and 'tk-s' not in a1 and 'tk-s' not in a2:
            # legacy order (font, x, y, text, color) -> reorder
            stats["B8:reorder"] = stats.get("B8:reorder", 0) + 1
            return '<span class="tk-f">xgeTextDraw</span>(%s, %s, %s, %s, %s)' % (a_font, a3, a1, a2, a_col)
        stats["B8:rename"] = stats.get("B8:rename", 0) + 1
        return '<span class="tk-f">xgeTextDraw</span>(%s)' % m.group(2)
    t = rep(t, r'<span class="tk-f">(xgeDrawText)</span>\((.*?)\);', b8, "B8:drawtext", flags=re.S)

    # B9 AppendRoundRect -> AppendRect (accept 6..8 args; pad rx/ry/cw as needed)
    def b9(m):
        args = split_args(m.group(1))
        if len(args) == 8:
            stats["B9:rename"] = stats.get("B9:rename", 0) + 1
            return '<span class="tk-f">xgeShapeExAppendRect</span>(%s)' % m.group(1)
        if len(args) == 7:  # shape,x,y,w,h,r -> add ry,cw
            stats["B9:pad2"] = stats.get("B9:pad2", 0) + 1
            return ('<span class="tk-f">xgeShapeExAppendRect</span>(%s, %s, <span class="tk-n">0</span>)'
                    % (m.group(1), args[6]))
        if len(args) == 6:  # shape,x,y,w,h,r -> rx=r,ry=r,cw=0
            stats["B9:append"] = stats.get("B9:append", 0) + 1
            return ('<span class="tk-f">xgeShapeExAppendRect</span>(%s, %s, %s, <span class="tk-n">0</span>)'
                    % (m.group(1), args[5], args[5]))
        stats["B9:SKIP-argc"] = stats.get("B9:SKIP-argc", 0) + 1
        return m.group(0)
    t = rep(t, r'<span class="tk-f">xgeShapeExAppendRoundRect</span>\((.*?)\);', b9, "B9:append", flags=re.S)

    # B10 rect fill/stroke arg surgery (5-arg fill -> RectFill; 6-arg -> RectStroke(rect,w,color))
    def b10(m):
        blob = m.group(1)
        args = split_args(blob)
        if len(args) == 5:
            stats["B10:rect"] = stats.get("B10:rect", 0) + 1
            return ('<span class="tk-f">xgeShapeRectFill</span>((<span class="tk-t">xge_rect_t</span>){%s, %s, %s, %s}, %s)'
                    % (args[0], args[1], args[2], args[3], args[4]))
        if len(args) == 6:
            stats["B10:stroke"] = stats.get("B10:stroke", 0) + 1
            return ('<span class="tk-f">xgeShapeRectStroke</span>((<span class="tk-t">xge_rect_t</span>){%s, %s, %s, %s}, %s, %s)'
                    % (args[0], args[1], args[2], args[3], args[5], args[4]))
        stats["B10:SKIP-argc"] = stats.get("B10:SKIP-argc", 0) + 1
        return m.group(0)
    t = rep(t, r'<span class="tk-f">(?:xgeShapeRect|xgeDrawRect)</span>\((.*?)\);', b10, "B10:rectfill", flags=re.S)

    # B11 mouse helpers -> xgeMouseDown(button-bit)
    for idx, bit in ((0, "XGE_MOUSE_LEFT"), (1, "XGE_MOUSE_RIGHT")):
        for fn in ("xgeMouseIsDown", "xgeMouseGetDown", "xgeMouseGetButton"):
            t = rep(t,
                    r'<span class="tk-f">%s</span>\(<span class="tk-n">%d</span>\)' % (fn, idx),
                    '<span class="tk-f">xgeMouseDown</span>(<span class="tk-m">%s</span>)' % bit,
                    "B11:down")
        t = rep(t,
                r'<span class="tk-f">xgeMouseGetUp</span>\(<span class="tk-n">%d</span>\)' % idx,
                '<span class="tk-k">!</span><span class="tk-f">xgeMouseDown</span>(<span class="tk-m">%s</span>)'
                ' <span class="tk-c">/* 释放边沿建议改用事件 */</span>' % bit,
                "B11:up")

    if t != orig:
        with open(p, "w", encoding="utf-8", newline="") as f:
            f.write(t)
        return True
    return False


def main():
    files = sorted(os.path.basename(p) for p in glob.glob(os.path.join(SITE, "ch*.html")))
    # int->float mouse vars in ch72/ch75 (xgeMouseGet takes float*)
    for fname in ("ch72.html", "ch75.html"):
        p = os.path.join(SITE, fname)
        t = open(p, encoding="utf-8", newline="").read()
        t2 = t.replace("<span class=\"tk-k\">int</span> mx, my;", "<span class=\"tk-k\">float</span> mx, my;")
        if t2 != t:
            open(p, "w", encoding="utf-8", newline="").write(t2)
            stats["A:mouse-int-float"] = stats.get("A:mouse-int-float", 0) + 1
    changed = sum(1 for f in files if transform(f))
    print("files changed: %d / %d" % (changed, len(files)))
    for k in sorted(stats):
        print("%-34s %d" % (k, stats[k]))


if __name__ == "__main__":
    main()
