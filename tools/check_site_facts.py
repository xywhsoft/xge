#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""check_site_facts.py - validate wwwroot site claims against engine source truth.

Usage:
    python tools/check_site_facts.py [--site D:/GIT/home/host/xge/wwwroot] [--repo D:/GIT/xge]

Prints ASCII PASS/FAIL lines; exit code 1 if any check fails.
Fact baseline lives in plans/xge-site-update/SPEC.md; re-measure before editing claims.
"""
import argparse
import glob
import os
import re
import sys


def read(path):
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        return f.read()


def wc_lines(paths):
    n = 0
    for p in paths:
        try:
            with open(p, "r", encoding="utf-8", errors="replace") as f:
                n += sum(1 for _ in f)
        except OSError:
            pass
    return n


def uniq(pattern, text):
    return sorted(set(re.findall(pattern, text)))


def main():
    ap = argparse.ArgumentParser()
    default_repo = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    ap.add_argument("--repo", default=default_repo)
    ap.add_argument("--site", default="D:/GIT/home/host/xge/wwwroot")
    a = ap.parse_args()
    repo, site = a.repo, a.site

    fails = []

    def check(name, ok, detail=""):
        print("[PASS] %s%s" % (name, (" - " + detail) if detail else "") if ok
              else "[FAIL] %s%s" % (name, (" - " + detail) if detail else ""))
        if not ok:
            fails.append(name)

    # ---------- measured source truth ----------
    xgeh = read(os.path.join(repo, "xge.h"))
    xuih = read(os.path.join(repo, "xui.h"))
    doch = read(os.path.join(repo, "xui_document.h")) + read(os.path.join(repo, "xui_document_ui.h"))
    doch_only = read(os.path.join(repo, "xui_document.h"))
    doc_api_count = sum(1 for l in doch_only.splitlines() if l.startswith("XUI_API "))
    headers_all = xgeh + "\n" + xuih + "\n" + doch

    m = re.search(r"#define XUI_PROXY_VERSION\s+(\d+)", xuih)
    proxy_ver = m.group(1) if m else "?"
    ver = ["%d" % int(x) for x in re.findall(
        r"#define XGE_VERSION_(?:MAJOR|MINOR|PATCH)\s+(\d+)", xgeh)]

    src_c_h = [p for p in glob.glob(os.path.join(repo, "src", "**", "*.[ch]"), recursive=True)]
    lib_c_h = [p for p in glob.glob(os.path.join(repo, "lib", "**", "*.[ch]"), recursive=True)]
    root_pub = [os.path.join(repo, f) for f in
                ("xge.h", "xui.h", "xui_document.h", "xui_document_ui.h",
                 "xge_config.h", "xui_config.h", "xge.c")]
    total_lines = wc_lines(src_c_h + lib_c_h + root_pub)
    total_k = int(total_lines // 10000) * 10  # display floors to 10K, e.g. 933747 -> 930

    svg_lines = wc_lines([os.path.join(repo, "src", "xge_svg.c")])
    svg_k = svg_lines // 1000  # display floors to K, e.g. 22221 -> 22
    shape_ex_lines = wc_lines(glob.glob(os.path.join(repo, "src", "xge_shape_ex*.c")))
    shape_ex_claim = int(round(shape_ex_lines / 100.0)) * 100

    widget_creates = uniq(r"\bxui[A-Z][A-Za-z0-9]*Create\b", xuih)
    example_dirs = sorted(d for d in os.listdir(os.path.join(repo, "examples"))
                          if os.path.isdir(os.path.join(repo, "examples", d)))
    test_files = glob.glob(os.path.join(repo, "test_xui", "*.c"))
    apis_3d = uniq(r"\bxge3d[A-Z][A-Za-z0-9]*\b", xgeh)

    chapters = [f for f in os.listdir(os.path.join(site, "tutorial"))
                if re.match(r"^ch\d+\.html$", f)]

    xge_h_lines = wc_lines([os.path.join(repo, "xge.h")])
    xui_h_lines = wc_lines([os.path.join(repo, "xui.h")])
    xrt_h_lines = wc_lines([os.path.join(repo, "lib", "xrt", "xrt.h")])

    profiles = ["default", "core", "2d", "3d", "ui-min", "full-dev", "ide"]
    bp = read(os.path.join(repo, "tools", "build_profile.py")) if os.path.exists(
        os.path.join(repo, "tools", "build_profile.py")) else ""

    scripts_needed = ["build_dll.bat", "build_dbg_dll.bat", "build_test.bat",
                      "build_android_ndk.bat", "build_android_apk.bat",
                      "build_ios_sim_exe.sh", "run_ios_sim_exe.sh", "serve_web_exe.bat",
                      "check_linux_egl_smoke.sh", "check_macos_smoke.sh",
                      "build_dll.sh", "build_test.sh", "ensure_xge_dll.bat"]

    print("== measured truth ==")
    print("  proxy=%s ver=%s total_lines=%d (%dK) svg=%d (%dK) shape_ex=%d (claim %d)"
          % (proxy_ver, ".".join(ver), total_lines, total_k, svg_lines, svg_k,
             shape_ex_lines, shape_ex_claim))
    print("  widgets(create)=%d examples=%d test_xui_c=%d apis3d=%d doc_apis=%d chapters=%d"
          % (len(widget_creates), len(example_dirs), len(test_files),
             len(apis_3d), doc_api_count, len(chapters)))
    print("  xge.h=%d xui.h=%d xrt.h=%d" % (xge_h_lines, xui_h_lines, xrt_h_lines))

    # ---------- per-page claims ----------
    idx = read(os.path.join(site, "index.html"))
    docs = read(os.path.join(site, "docs", "index.html"))
    dl = read(os.path.join(site, "download", "index.html"))
    ex = read(os.path.join(site, "examples", "index.html"))
    tut = read(os.path.join(site, "tutorial", "index.html"))
    ch01 = read(os.path.join(site, "tutorial", "ch01.html"))

    check("version badge v%s" % ".".join(ver), ("v%s" % ".".join(ver)) in idx)

    m = re.search(r'<span data-count="(\d+)" data-suffix="K">', idx)
    check("stats total K", m and int(m.group(1)) == total_k,
          "site=%s truth=%dK" % (m.group(1) if m else "?", total_k))

    m = re.search(r'data-count="(\d+)"[^>]*>\s*0</span><i>\+</i></b><span>UI', idx)
    if not m:
        m = re.search(r'data-count="(\d+)"[^>]*>0</span><i>\+</i></b><span>\s*(?:UI )?控件', idx)
    check("stats widgets", m and int(m.group(1)) == len(widget_creates),
          "site=%s truth=%d" % (m.group(1) if m else "?", len(widget_creates)))

    m = re.search(r'data-count="(\d+)"[^>]*>0</span></b><span>控件测试文件', idx)
    check("stats test files", m and int(m.group(1)) == len(test_files),
          "site=%s truth=%d" % (m.group(1) if m else "?", len(test_files)))

    m = re.search(r'data-count="(\d+)"[^>]*>0</span></b><span>行 SVG 引擎', idx)
    check("stats svg K", m and int(m.group(1)) == svg_k,
          "site=%s truth=%dK" % (m.group(1) if m else "?", svg_k))

    m = re.search(r'data-count="(\d+)"[^>]*>0</span>(?:<i>\+</i>)?</b><span>3D', idx)
    check("stats 3d apis", m and int(m.group(1)) == len(apis_3d),
          "site=%s truth=%d" % (m.group(1) if m else "?", len(apis_3d)))

    check("index shape_ex lines", ("%d" % shape_ex_claim) in idx or
          ("{:,}".format(shape_ex_claim)) in idx,
          "expect %d" % shape_ex_claim)

    check("index examples count", ("%d" % len(example_dirs)) in idx,
          "expect %d" % len(example_dirs))

    check("download proxy version",
          ("XUI_PROXY_VERSION %s" % proxy_ver) in dl, "expect %s" % proxy_ver)

    check("github clone url kept", "https://github.com/xywhsoft/xge" in dl)
    check("gitee mirror url", "https://gitee.com/xywhsoft/xge" in dl)

    n227 = len(chapters)
    check("tutorial chapter count copy",
          tut.count("%d 章" % n227) >= 2 and ("%d Chapters" % n227) in tut,
          "expect %d" % n227)
    check("tutorial meta description count",
          ("%d 章" % n227) in re.search(r'name="description" content="([^"]*)"',
                                        tut).group(1))

    check("ch01 header line counts",
          all(s in ch01 for s in
              ["{:,}".format(xge_h_lines), "{:,}".format(xui_h_lines),
               "{:,}".format(xrt_h_lines)]),
          "expect %s/%s/%s" % (xge_h_lines, xui_h_lines, xrt_h_lines))

    # docs page: document widget api claim
    m = re.search(r'(\d+)\s*个.*xuiDocument|xuiDocument[^\n]{0,40}?(\d+)', docs)
    check("docs document api count",
          bool(m) and any(int(g) == doc_api_count for g in m.groups() if g),
          "expect %d" % doc_api_count)

    # build scripts existence + profiles
    missing = [s for s in scripts_needed
               if not os.path.exists(os.path.join(repo, s))]
    check("build scripts exist", not missing, "missing=%s" % ",".join(missing))
    check("profiles in build_profile.py",
          all(("'%s'" % p) in bp or ('"%s"' % p) in bp for p in profiles))

    # ---------- site-wide API name existence ----------
    # Any xge*/xui*/XGE_*/XUI_* identifier on checked pages must appear in the
    # public headers, or in the whitelist (paths, dirs, file names).
    # Scope: ALL pages + every chapter (legacy debt remediated 2026-10-04, M7).
    whitelist = set(["xge", "xui", "XGE", "XUI", "xge3d", "XGE3D",
                     "xge_impl", "xrt", "xge_build_config"])
    for d in example_dirs:
        whitelist.add(d)
    for extra in ("xge_shape_ex", "xge_demo", "xge_test", "xgedbg", "libxge",
                  "test_xui", "build_xge", "xge_dll", "xui_dll", "main", "onFrame"):
        whitelist.add(extra)
    declared = set(re.findall(r"[A-Za-z_][A-Za-z0-9_]*", headers_all))
    family = set(["xuiDocument", "xgeShape", "xgeSvg", "xgeShapeEx", "xuiInput", "xuiInputPointer",
                  "xuiCanvas", "xuiDockPanel", "xuiPropertyGrid", "xuiToolbar", "xuiTreeView",
                  "xuiTimeLineView", "xuiChart", "xuiMsgBox", "xuiTableGrid"])

    def token_ok(tok):
        if tok in whitelist or tok in declared or tok in family or tok.endswith("_"):
            return True
        return any(len(d) > len(tok) and d.startswith(tok)
                   and (d[len(tok)] == "_" or d[len(tok)].islower()) for d in declared)
    def chnum(f):
        m = re.match(r"ch(\d+)\.html$", f)
        return int(m.group(1)) if m else None
    tut_dir = os.path.join(site, "tutorial")
    scope = [idx, docs, dl, ex, tut, ch01]
    legacy = 0
    for f in os.listdir(tut_dir):
        n = chnum(f)
        if n is None:
            continue
        scope.append(read(os.path.join(tut_dir, f)))

    bad = {}
    for page in scope:
        for tok in uniq(r"\b(?:xge3d[A-Za-z0-9_]+|xge[A-Za-z0-9_]+|xui[A-Za-z0-9_]+"
                        r"|XGE3D_[A-Z0-9_]+|XGE_[A-Z0-9_]+|XUI_[A-Z0-9_]+)\b", page):
            if token_ok(tok):
                continue
            bad.setdefault(tok, 0)
            bad[tok] += 1
    print("  (name gate covers all %d chapters)" % (len(chapters)))
    check("site-wide api names exist in headers", not bad,
          "; ".join("%s(x%d)" % (k, v) for k, v in sorted(bad.items())[:12]))

    # ---------- summary ----------
    print("== %d failed ==" % len(fails))
    if fails:
        print("FAILED: %s" % ", ".join(fails))
        return 1
    print("ALL GREEN")
    return 0


if __name__ == "__main__":
    sys.exit(main())
