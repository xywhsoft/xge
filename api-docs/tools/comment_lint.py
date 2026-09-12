#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""comment_lint.py —— 注释门禁（SPEC 第 8 节四条规则）

  1. 新增 API 无注释            → FAIL（对比 coverage.json 基线中不存在的 API）
  2. 覆盖率棘轮下降              → FAIL（当前已注释数低于基线记录值）
  3. 注释与声明间隔 >1 空行      → 告警（不判 FAIL）
  4. verify_batch.bat 语法失败   → FAIL（仅 --verify 时执行）

用法：
  python comment_lint.py --report          # 报告模式，只打印不判失败
  python comment_lint.py                   # 门禁模式（基线缺失时自动降级为报告）
  python comment_lint.py --verify          # 附加执行 verify_batch.bat
"""
import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import api_comment_coverage as cov

BASELINE = cov.API_DOCS / "coverage.json"


def gap_warnings(header: str):
    """规则 3：注释与声明之间空行 >1。返回 [api_name]。"""
    path = cov.REPO / header
    lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    marker = cov.HEADERS[header][0]
    warns = []
    for i, line in enumerate(lines):
        if marker not in line:
            continue
        m = re.search(cov.HEADERS[header][1] + r"[A-Za-z_][A-Za-z0-9_]*", line)
        if not m:
            continue
        # 向上找注释（最多 5 行），统计其间空行数
        blanks = 0
        found = False
        for j in range(i - 1, max(-1, i - 6), -1):
            s = lines[j].strip()
            if s == "":
                blanks += 1
                continue
            if "/*" in s or s.startswith("*"):
                found = True
            break
        if found and blanks > 1:
            warns.append(m.group(0))
    return warns


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--report", action="store_true", help="报告模式：只打印，不返回失败码")
    ap.add_argument("--verify", action="store_true", help="执行 verify_batch.bat（规则 4）")
    args = ap.parse_args()

    if not BASELINE.exists():
        print("[WARN] coverage.json baseline missing; run api_comment_coverage.py first. Report-only mode.")
        args.report = True

    baseline = json.loads(BASELINE.read_text(encoding="utf-8")) if BASELINE.exists() else None
    fails, warns = [], []

    for header in cov.HEADERS:
        apis = cov.parse_header(cov.REPO / header)
        seen = set()
        unique = []
        for a in apis:
            if a[0] not in seen:
                seen.add(a[0])
                unique.append(a)
        cur_doc = {a[0] for a in unique if a[2]}
        print(f"== {header}: {len(cur_doc)}/{len(unique)} documented")

        if baseline:
            base_all = set(baseline["all_apis"].get(header, []))
            base_doc = len(baseline["documented_apis"].get(header, []))
            # 规则 1：新增 API 必须带注释
            for name, _ln, doc in unique:
                if name not in base_all and not doc:
                    fails.append(f"gate1 {header}: new API without comment -> {name}")
            # 规则 2：棘轮
            if len(cur_doc) < base_doc:
                fails.append(f"gate2 {header}: documented {len(cur_doc)} < baseline {base_doc}")
            elif len(cur_doc) > base_doc:
                print(f"   [ratchet] coverage up {base_doc} -> {len(cur_doc)}, baseline can be refreshed")
        # 规则 3：间隔告警
        for w in gap_warnings(header):
            warns.append(f"gate3 {header}: comment gap >1 blank line -> {w}")

    if args.verify:
        bat = Path(__file__).parent / "verify_batch.bat"
        r = subprocess.run(["cmd", "/c", str(bat)], capture_output=True, text=True)
        print(r.stdout.strip())
        if r.returncode != 0:
            fails.append("gate4 verify_batch.bat syntax check failed")

    for w in warns:
        print(f"[WARN] {w}")
    if fails:
        for f in fails:
            print(f"[FAIL] {f}")
        if not args.report:
            return 1
        print("(report mode: failures above do not set exit code)")
    else:
        print("[OK] gates passed" + (" (report mode)" if args.report else ""))
    return 0


if __name__ == "__main__":
    sys.exit(main())
