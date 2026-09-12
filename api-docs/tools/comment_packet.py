#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""comment_packet.py —— 按模块生成 API 注释草稿包（SPEC 第 5 节取证规则）

每条 API 输出：签名 / 头文件位置 / 上方既有注释 / 实现定位（src/*.c 列 0
定义式启发式）/ 实现体内 return 错误码清单 / examples、test、test_xui、
tutorial_capture 中的用法行号（每条最多 3 处）。

用法：
  python comment_packet.py --header xge --module Svg
  python comment_packet.py --header xui --module button
  python comment_packet.py --header xge --list
输出：api-docs/packets/<header>_<module>.md（模块名中的非文件名字符会被替换）。
只读分析，不修改仓库文件。
"""
import argparse
import re
import sys
from datetime import datetime
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import api_comment_coverage as cov

REPO = cov.REPO
PACKETS = cov.API_DOCS / "packets"
USAGE_ROOTS = ["examples", "test", "test_xui"]
IMPL_GLOB = "src/*.c"


def load_header_apis(header: str):
    """返回 [(name, line1b, documented, signature, existing_comment)]。"""
    path = REPO / header
    marker, prefix = cov.HEADERS[header]
    lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    out = []
    for i, line in enumerate(lines):
        if marker not in line:
            continue
        # 与 api_comment_coverage.parse_header 相同：优先取 "(" 前的函数名，
        # 防止把返回类型（xge_camera_t / xui_widget 等）误当函数名
        m = re.search(prefix + r"[A-Za-z0-9_]*\s*\(", line)
        if m:
            name = re.match(prefix + r"[A-Za-z0-9_]*", m.group(0)).group(0)
        else:
            m = re.search(prefix + r"[A-Za-z_][A-Za-z0-9_]*", line)
            if not m:
                continue
            name = m.group(0)
        sig = line.strip()
        k = i
        while ";" not in sig and k + 1 < len(lines) and k - i < 6:
            k += 1
            sig += " " + lines[k].strip()
        # 既有注释（向上跳过至多 1 空行，最多回看 6 行收齐多行块）
        comment = []
        j = i - 1
        blanks = 0
        while j >= 0:
            s = lines[j].strip()
            if s == "":
                blanks += 1
                if blanks > 1:
                    break
                j -= 1
                continue
            if "/*" in s or s.startswith("*"):
                comment.insert(0, s)
                j -= 1
                continue
            break
        out.append((name, i + 1, cov.documented(lines, i), " ".join(sig.split()),
                    " ".join(" ".join(comment).split())))
    return out


def find_implementations(names):
    """col-0 定义式启发式：返回 {name: (file, line1b, body_lines)}。"""
    pats = {n: re.compile(r"^[A-Za-z_][\w \t\*]*\b" + re.escape(n) + r"\s*\(")
            for n in names}
    result = {}
    for f in sorted(REPO.glob(IMPL_GLOB)):
        try:
            lines = f.read_text(encoding="utf-8", errors="replace").splitlines()
        except OSError:
            continue
        for idx, line in enumerate(lines):
            for n, pat in pats.items():
                if n in result or not pat.match(line):
                    continue
                body = [line]
                j = idx + 1
                while j < len(lines) and lines[j].rstrip() != "}":
                    body.append(lines[j])
                    j += 1
                    if j - idx > 3000:
                        break
                if j < len(lines):
                    body.append("}")
                result[n] = (str(f.relative_to(REPO)).replace("\\", "/"),
                             idx + 1, body)
    return result


def return_codes(body_lines):
    codes = []
    for line in body_lines:
        for m in re.finditer(r"return\s+([A-Za-z_][A-Za-z0-9_]*|-?\d+)", line):
            tok = m.group(1)
            if tok.startswith(("XGE_", "XUI_")) or tok in ("true", "false", "NULL"):
                if tok not in codes:
                    codes.append(tok)
    return codes


def usage_refs(names, cap=3):
    """在 examples/test/test_xui 中扫描模块 API 用法，返回 {name: [file:line]}。"""
    alt = "|".join(re.escape(n) for n in names)
    pat = re.compile(r"\b(" + alt + r")\s*\(")
    refs = {n: [] for n in names}
    for root in USAGE_ROOTS:
        base = REPO / root
        if not base.is_dir():
            continue
        for f in base.rglob("*.c"):
            try:
                lines = f.read_text(encoding="utf-8", errors="replace").splitlines()
            except OSError:
                continue
            rel = str(f.relative_to(REPO)).replace("\\", "/")
            for idx, line in enumerate(lines):
                stripped = line.strip()
                if len(stripped) < 4 or stripped.startswith(("*", "//", "/*")):
                    continue
                for m in pat.finditer(line):
                    n = m.group(1)
                    if len(refs[n]) < 8:
                        refs[n].append(f"{rel}:{idx + 1}")
    return {n: v[:cap] for n, v in refs.items()}


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--header", choices=["xge", "xui", "xge.h", "xui.h"])
    ap.add_argument("--module")
    ap.add_argument("--list", action="store_true", help="列出该头文件全部模块及条数")
    args = ap.parse_args()
    if not args.header:
        print("need --header xge|xui", file=sys.stderr)
        return 2
    header = args.header if args.header.endswith(".h") else args.header + ".h"
    apis = load_header_apis(header)
    if args.list:
        mods = {}
        for name, _ln, _d, _s, _c in apis:
            mod = cov.module_of(name, cov.HEADERS[header][1])[0]
            mods.setdefault(mod, []).append(name)
        for mod, names in sorted(mods.items(), key=lambda kv: -len(kv[1])):
            print(f"{mod:<20} {len(names):>4}")
        return 0
    if not args.module:
        print("need --module <name> (or --list)", file=sys.stderr)
        return 2

    target = [a for a in apis if cov.module_of(a[0], cov.HEADERS[header][1])[0] == args.module]
    if not target:
        print(f"module {args.module} has no API in {header}", file=sys.stderr)
        return 2
    names = [a[0] for a in target]
    impls = find_implementations(names)
    refs = usage_refs(names)

    PACKETS.mkdir(parents=True, exist_ok=True)
    safe = re.sub(r"[^A-Za-z0-9_-]", "_", args.module)
    out = PACKETS / f"{header.removesuffix('.h')}_{safe}.md"
    with out.open("w", encoding="utf-8", newline="\n") as fp:
        fp.write(f"# 草稿包：{header} / {args.module}（{len(names)} 条 API）\n\n")
        fp.write(f"> 生成 {datetime.now():%Y-%m-%d %H:%M} | revision {cov.git_revision()[:12]}\n")
        fp.write("> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；"
                 "平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。\n\n")
        for name, ln, doc, sig, comment in target:
            fp.write(f"## {name}\n")
            fp.write(f"- 位置: {header}:{ln}  已注释: {'是' if doc else '否'}\n")
            fp.write(f"- 签名: `{sig}`\n")
            if comment:
                fp.write(f"- 既有注释: {comment}\n")
            if name in impls:
                f, l, body = impls[name]
                codes = return_codes(body)
                fp.write(f"- 实现: {f}:{l}（体 {len(body)} 行）\n")
                if codes:
                    fp.write(f"- 返回码: {', '.join(codes)}\n")
            else:
                fp.write("- 实现: 未定位（可能在条件编译块或别名定义，需人工确认）\n")
            rs = refs.get(name, [])
            if rs:
                fp.write(f"- 用法: {'; '.join(rs)}\n")
            fp.write("\n")
    print(f"[OK] {out} ({len(names)} APIs)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
