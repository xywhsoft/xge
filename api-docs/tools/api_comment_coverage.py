#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""api_comment_coverage.py —— XGE API 注释覆盖率统计（SPEC 第 2/6/8 节依据）

判定规则（SPEC 第 6 节）：API 声明行（含 XGE_API / XUI_API 的行）上方，
向上跳过至多 1 个空行后的第一条非空行是 /* */ 块注释（含多行注释的
续行，即以 * 开头或含 /* 的行）→ 视为已注释；若先遇到另一条声明或
宏行则视为未注释（归属消歧，防止把上一条 API 的注释记到本条头上）。

模块聚类：取 API 名去掉 xge/xui 前缀后的驼峰首词（Shape+Ex 合并为
ShapeEx；小写开头词如 dbg 单独成族）。xui.h 额外按 58 控件名单
（与 PROGRESS.md 第 3.3 节同步维护）切分控件族 / 框架族。

输出：api-docs/coverage.json + stdout 摘要表。
只读分析，不修改任何文件。
"""
import json
import re
import subprocess
import sys
from datetime import datetime
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
API_DOCS = REPO / "api-docs"
HEADERS = {
    "xge.h": ("XGE_API", "xge"),
    "xui.h": ("XUI_API", "xui"),
}

# 58 个控件类型名（来源：grep -h "tDesc.sName" src/xui_*.c，见 PROGRESS.md 3.3）
WIDGETS = [
    "RichEdit", "accordion", "breadcrumb", "button", "canvas", "carousel",
    "cascader", "chart", "checkbox", "checkcard", "codeedit", "colorpicker",
    "combobox", "datepicker", "flowgraph", "hyperlink", "iconpicker", "image",
    "input", "inventorygrid", "label", "listview", "menu", "menubar",
    "messagelist", "msgtip", "numeric_input", "page", "panel", "popup",
    "progress", "propertygrid", "qrcode", "radio", "radiogroup", "rangeslider",
    "scrollbar", "scrollframe", "scrollview", "separator", "slider",
    "splitlayout", "statusbar", "stepbar", "tablegrid", "tableview", "tabs",
    "taginput", "terminal", "textedit", "timelineview", "toast-item", "toggle",
    "toolbar", "treeview", "virtual_joystick", "window", "workflow",
]
# 控件名规范化（去 - 和 _、转小写）→ 控件 API 前缀为单词时的别名
WIDGET_ALIAS = {"toast": "toast-item"}


def _norm(name: str) -> str:
    return name.replace("-", "").replace("_", "").lower()


WIDGET_NORM = {_norm(w): w for w in WIDGETS}


def camel_tokens(body: str):
    """去前缀后的函数体驼峰切词；小写开头词（如 dbg）也作为独立词。"""
    return re.findall(r"[a-z]+|[A-Z][a-z0-9]*", body)


def module_of(name: str, prefix: str) -> tuple:
    """返回 (module, is_widget, widget_name)。"""
    body = name[len(prefix):]
    tokens = camel_tokens(body)
    if not tokens:
        return ("core", False, None)
    if prefix == "xge":
        if len(tokens) >= 2 and tokens[0] == "Shape" and tokens[1] == "Ex":
            return ("ShapeEx", False, None)
        return (tokens[0], False, None)
    # xui：尝试 1~3 个词拼接匹配控件名（词数不足时 continue 而非 break，
    # 保证短名 API 仍能用 k=1 命中，如 xuiToastShow -> toast-item）
    for k in (3, 2, 1):
        if len(tokens) < k:
            continue
        joined = "".join(tokens[:k]).lower()
        if joined in WIDGET_NORM:
            return (WIDGET_NORM[joined], True, WIDGET_NORM[joined])
        if k == 1 and joined in WIDGET_ALIAS:
            w = WIDGET_ALIAS[joined]
            return (w, True, w)
    return (tokens[0], False, None)


def documented(lines, idx: int) -> bool:
    """声明行 idx 上方是否挂有块注释（见文件头判定规则）。"""
    blanks = 0
    j = idx - 1
    while j >= 0:
        s = lines[j].strip()
        if s == "":
            blanks += 1
            if blanks > 1:
                return False
            j -= 1
            continue
        return ("/*" in s) or s.startswith("*")
    return False


def parse_header(path: Path):
    """返回 [(name, line_no_1based, documented_bool)]。"""
    marker, prefix = HEADERS[path.name]
    lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    out = []
    for i, line in enumerate(lines):
        if marker not in line:
            continue
        # 优先取紧跟 "(" 的前缀标识符（函数名），防止把返回类型（如
        # xge_camera_t / xui_widget）误当函数名；多行声明回退首个前缀标识符
        m = re.search(prefix + r"[A-Za-z0-9_]*\s*\(", line)
        if m:
            name = re.match(prefix + r"[A-Za-z0-9_]*", m.group(0)).group(0)
        else:
            m = re.search(prefix + r"[A-Za-z_][A-Za-z0-9_]*", line)
            if not m:
                continue
            name = m.group(0)
        out.append((name, i + 1, documented(lines, i)))
    return out


def git_revision() -> str:
    try:
        return subprocess.run(
            ["git", "rev-parse", "HEAD"], cwd=REPO, capture_output=True,
            text=True, timeout=10).stdout.strip()
    except Exception:
        return ""


def build_report() -> dict:
    report = {
        "generated_at": datetime.now().isoformat(timespec="seconds"),
        "git_revision": git_revision(),
        "rule": "decl-attached block comment within 1 blank line above (SPEC 6)",
        "files": {},
        "documented_apis": {},
        "all_apis": {},
    }
    for fname in HEADERS:
        apis = parse_header(REPO / fname)
        names = [a[0] for a in apis]
        # 同名重复声明按首现计
        seen, unique = set(), []
        for a in apis:
            if a[0] not in seen:
                seen.add(a[0])
                unique.append(a)
        mods, wtot, wdoc, ftot, fdoc = {}, 0, 0, 0, 0
        for name, _ln, doc in unique:
            mod, is_w, _wname = module_of(name, HEADERS[fname][1])
            m = mods.setdefault(mod, {"total": 0, "documented": 0})
            m["total"] += 1
            m["documented"] += 1 if doc else 0
            if is_w:
                wtot += 1
                wdoc += 1 if doc else 0
            else:
                ftot += 1
                fdoc += 1 if doc else 0
        entry = {
            "total_marker_lines": len(apis),
            "total_unique": len(unique),
            "documented": sum(1 for a in unique if a[2]),
            "rate": round(sum(1 for a in unique if a[2]) / len(unique), 4) if unique else 0,
            "modules": dict(sorted(mods.items(), key=lambda kv: -kv[1]["total"])),
        }
        if fname == "xui.h":
            entry["widget_apis"] = {"total": wtot, "documented": wdoc}
            entry["framework_apis"] = {"total": ftot, "documented": fdoc}
        report["files"][fname] = entry
        report["documented_apis"][fname] = [a[0] for a in unique if a[2]]
        report["all_apis"][fname] = [a[0] for a in unique]
    return report


def main() -> int:
    report = build_report()
    out = API_DOCS / "coverage.json"
    out.write_text(json.dumps(report, ensure_ascii=False, indent=1), encoding="utf-8")
    for fname, e in report["files"].items():
        print(f"== {fname}: {e['documented']}/{e['total_unique']} documented "
              f"({e['rate']*100:.1f}%)  [marker lines {e['total_marker_lines']}]")
        if "widget_apis" in e:
            w, f = e["widget_apis"], e["framework_apis"]
            print(f"   widgets  : {w['documented']}/{w['total']}")
            print(f"   framework: {f['documented']}/{f['total']}")
        for mod, m in e["modules"].items():
            print(f"   {mod:<16} {m['documented']:>4}/{m['total']:<4}")
    print(f"[OK] coverage.json -> {out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
