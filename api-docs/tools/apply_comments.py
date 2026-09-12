#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""apply_comments.py —— 把注释批量插入头文件（M1-M3 批处理用）

输入：JSON 文件 { "API 名": ["/* 注释行1", " * 注释行2 */"], ... }
行为：按函数名定位声明行（与 api_comment_coverage 相同的提取规则），
在声明上方插入注释行；已有注释的声明跳过（幂等）；找不到的名单报告。
行尾保持原文件风格（CRLF）；写出为 UTF-8（与 src/xge_impl.c 先例一致）。
不改任何既有行。用法：
  python apply_comments.py --header xge.h --comments packets/batch_svg.json
  python apply_comments.py --header xge.h --comments xx.json --dry
"""
import argparse
import json
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import api_comment_coverage as cov


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--header", required=True, choices=["xge.h", "xui.h"])
    ap.add_argument("--comments", required=True)
    ap.add_argument("--dry", action="store_true")
    args = ap.parse_args()

    comments = json.loads(Path(args.comments).read_text(encoding="utf-8"))
    path = cov.REPO / args.header
    raw = path.read_bytes().decode("utf-8")
    # 原文件为 CRLF/LF 混合行尾：逐行保留原行尾，禁止按单一行尾分割
    # （按单一 "\r\n" split 会把 LF 段的相邻声明合并成一行，导致名字定位失败）
    parts = re.split(r"(\r\n|\n|\r)", raw)
    lines, seps = [], []
    for j in range(0, len(parts), 2):
        lines.append(parts[j])
        seps.append(parts[j + 1] if j + 1 < len(parts) else "")
    marker, prefix = cov.HEADERS[args.header]

    # 定位每个 API 的首个声明行
    loc = {}
    for i, line in enumerate(lines):
        if marker not in line:
            continue
        m = re.search(prefix + r"[A-Za-z0-9_]*\s*\(", line)
        if not m:
            continue
        name = re.match(prefix + r"[A-Za-z0-9_]*", m.group(0)).group(0)
        loc.setdefault(name, i)

    applied, skipped, missing = 0, [], []
    inserts = []  # (idx, [lines])
    for name, block in comments.items():
        if name not in loc:
            missing.append(name)
            continue
        idx = loc[name]
        if cov.documented(lines, idx):
            skipped.append(name)
            continue
        block = [b.rstrip() for b in block if b.strip()]
        if not block:
            missing.append(name + "(empty)")
            continue
        inserts.append((idx, block))

    # 降序插入，避免行号失效；插入行沿用目标声明行的原行尾
    for idx, block in sorted(inserts, key=lambda t: -t[0]):
        sep = seps[idx] or "\r\n"
        for b in reversed(block):
            lines.insert(idx, b)
            seps.insert(idx, sep)
        applied += 1

    print(f"applied={applied} skipped(already)={len(skipped)} missing={len(missing)}")
    if missing:
        print("MISSING:", ", ".join(missing))
    if skipped and len(skipped) <= 10:
        print("SKIPPED:", ", ".join(skipped))
    if not args.dry:
        path.write_bytes("".join(l + s for l, s in zip(lines, seps)).encode("utf-8"))
        print(f"[OK] written {path}")
    else:
        print("[DRY] not written")
    return 1 if missing else 0


if __name__ == "__main__":
    sys.exit(main())
